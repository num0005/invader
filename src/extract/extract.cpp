// SPDX-License-Identifier: GPL-3.0-only

#include <optional>
#include <filesystem>
#include <invader/map/map.hpp>
#include <invader/file/file.hpp>
#include "../command_line_option.hpp"
#include <invader/crc/hek/crc.hpp>
#include <invader/version.hpp>
#include <invader/extract/extraction.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/tag/parser/parser.hpp>
#include <regex>

int main(int argc, const char **argv) {
    set_up_color_term();
    
    using namespace Invader;

    // Options struct
    struct ExtractOptions {
        std::optional<std::string> tags_directory;
        std::optional<std::string> maps_directory;
        std::vector<std::string> tags_to_extract;
        
        std::vector<std::string> search_queries;
        std::vector<std::string> search_queries_exclude;
        
        bool recursive = false;
        bool overwrite = false;
        bool non_mp_globals = false;
        bool ignore_resource_maps = false;
    } extract_options;

    // Command line options
    const CommandLineOption options[] {
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_INFO),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_MAPS),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_TAGS),
        CommandLineOption("recursive", 'r', 0, "Extract tag dependencies"),
        CommandLineOption("overwrite", 'O', 0, "Overwrite tags if they already exist"),
        CommandLineOption("ignore-resources", 'G', 0, "Ignore resource maps."),
        CommandLineOption("search", 's', 1, "Search for tags (* and ? are wildcards) and extract these. Use multiple times for multiple queries. If unspecified, all tags will be extracted.", "<expr>"),
        CommandLineOption("search-exclude", 'e', 1, "Search for tags (* and ? are wildcards) and ignore these. Use multiple times for multiple queries. This takes precedence over --search.", "<expr>"),
        CommandLineOption("non-mp-globals", 'n', 0, "Enable extraction of non-multiplayer .globals")
    };

    static constexpr char DESCRIPTION[] = "Extract data from cache files.";
    static constexpr char USAGE[] = "[options] <map>";

    // Do it!
    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<ExtractOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, extract_options, [](char opt, const auto &args, auto &extract_options) {
        switch(opt) {
            case 'I':
                extract_options.ignore_resource_maps = true;
                break;
            case 'm':
                extract_options.maps_directory = args[0];
                break;
            case 't':
                if(extract_options.tags_directory.has_value()) {
                    eprintf_error("This tool does not support multiple tags directories.");
                    std::exit(EXIT_FAILURE);
                }
                extract_options.tags_directory = args[0];
                break;
            case 'r':
                extract_options.recursive = true;
                break;
            case 'O':
                extract_options.overwrite = true;
                break;
            case 'n':
                extract_options.non_mp_globals = true;
                break;
            case 's':
                extract_options.search_queries.emplace_back(File::preferred_path_to_halo_path(args[0]));
                break;
            case 'e':
                extract_options.search_queries_exclude.emplace_back(File::preferred_path_to_halo_path(args[0]));
                break;
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
        }
    });


    if(!extract_options.tags_directory.has_value()) {
        extract_options.tags_directory = "tags";
    }

    // Check if the tags directory exists
    std::filesystem::path tags(*extract_options.tags_directory);
    if(!std::filesystem::is_directory(tags)) {
        if(extract_options.tags_directory == "tags") {
            eprintf_error("No tags directory was given, and \"tags\" was not found or is not a directory.");
        }
        else {
            eprintf_error("Directory %s was not found or is not a directory", extract_options.tags_directory->c_str());
        }
        return EXIT_FAILURE;
    }

    std::vector<std::byte> loc, bitmaps, sounds;

    // Find the asset data
    if(!extract_options.maps_directory.has_value()) {
        std::filesystem::path map = std::string(remaining_arguments[0]);
        auto maps_folder = std::filesystem::absolute(map).parent_path();
        if(std::filesystem::is_directory(maps_folder)) {
            extract_options.maps_directory = maps_folder.string();
        }
    }

    // Load resource maps
    if(extract_options.maps_directory.has_value() && !extract_options.ignore_resource_maps) {
        std::filesystem::path maps_directory(*extract_options.maps_directory);
        auto open_map_possibly = [&maps_directory](const char *map) -> std::vector<std::byte> {
            auto potential_map = Invader::File::open_file(maps_directory / map);
            if(potential_map.has_value()) {
                return *potential_map;
            }
            else {
                return std::vector<std::byte>();
            }
        };

        // Get its header
        Invader::HEK::CacheFileHeader header;
        std::FILE *f = std::fopen(remaining_arguments[0], "rb");
        if(!f) {
            eprintf_error("Failed to open %s to determine its version", remaining_arguments[0]);
            return EXIT_FAILURE;
        }
        if(!std::fread(&header, sizeof(header), 1, f)) {
            eprintf_error("Failed to read %s to determine its version", remaining_arguments[0]);
            std::fclose(f);
            return EXIT_FAILURE;
        }
        std::fclose(f);

        // Check if we can do things to it
        if(header.valid()) {
            switch(header.engine.read()) {
                case HEK::CACHE_FILE_DEMO:
                case HEK::CACHE_FILE_RETAIL:
                    bitmaps = open_map_possibly("bitmaps.map");
                    sounds = open_map_possibly("sounds.map");
                    break;
                case HEK::CACHE_FILE_MCC_CEA:
                    if(!(reinterpret_cast<const HEK::CacheFileHeaderCEA *>(&header)->flags & HEK::CacheFileHeaderCEAFlags::CACHE_FILE_HEADER_CEA_FLAGS_CLASSIC_ONLY)) {
                        bitmaps = open_map_possibly("bitmaps.map");
                    }
                    break;
                case HEK::CACHE_FILE_CUSTOM_EDITION:
                    loc = open_map_possibly("loc.map");
                    bitmaps = open_map_possibly("bitmaps.map");
                    sounds = open_map_possibly("sounds.map");
                    break;
                default:
                    break; // nothing else gets resource maps
            }
        }
        // Maybe it's a demo map?
        else if(reinterpret_cast<Invader::HEK::CacheFileDemoHeader *>(&header)->valid()) {
            bitmaps = open_map_possibly("bitmaps.map");
            sounds = open_map_possibly("sounds.map");
        }
    }

    // Load map
    std::unique_ptr<Map> map;
    try {
        auto file = File::open_file(remaining_arguments[0]).value();
        map = std::make_unique<Map>(Map::map_with_move(std::move(file), std::move(bitmaps), std::move(loc), std::move(sounds)));
    }
    catch (std::exception &e) {
        eprintf_error("Failed to parse %s: %s", remaining_arguments[0], e.what());
        return EXIT_FAILURE;
    }

    ExtractionWorkload::extract_map(*map, *extract_options.tags_directory, extract_options.search_queries, extract_options.search_queries_exclude, extract_options.recursive, extract_options.overwrite, extract_options.non_mp_globals);
}
