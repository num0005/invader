// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    static float get_bitmap_tag_pixel_size(BuildWorkload &workload, std::size_t bitmap_tag_index, std::size_t sequence_index, std::size_t sequence_count, bool &warn) {
        warn = false;
        
        if(workload.disable_recursion) {
            return 1.0F;
        }

        auto &bitmap_tag_struct = workload.structs[*workload.tags[bitmap_tag_index].base_struct];
        auto &bitmap_tag_data = *reinterpret_cast<Bitmap::struct_little *>(bitmap_tag_struct.data.data());
        float pixel_size = 1.0F;

        // Get the dimensions of the bitmaps
        std::uint32_t bitmap_count = bitmap_tag_data.bitmap_data.count;
        std::optional<std::size_t> bitmap_data_index = bitmap_tag_struct.resolve_pointer(&bitmap_tag_data.bitmap_data.pointer);

        if(bitmap_data_index.has_value()) {
            std::vector<std::pair<std::uint16_t, std::uint16_t>> bitmap_dimensions(bitmap_count);
            auto *bitmap_data = reinterpret_cast<BitmapData::struct_little *>(workload.structs[*bitmap_data_index].data.data());
            for(std::uint32_t b = 0; b < bitmap_count; b++) {
                bitmap_dimensions[b].first = bitmap_data[b].width;
                bitmap_dimensions[b].second = bitmap_data[b].height;
            }

            // Get sequences
            auto sequence_offset = bitmap_tag_struct.resolve_pointer(&bitmap_tag_data.bitmap_group_sequence.pointer);
            auto bitmap_sequence_count = static_cast<std::size_t>(bitmap_tag_data.bitmap_group_sequence.count);
            
            if(sequence_offset.has_value() && sequence_index < bitmap_tag_data.bitmap_group_sequence.count) {
                auto &sequences_struct = workload.structs[*sequence_offset];
                auto *sequences = reinterpret_cast<BitmapGroupSequence::struct_little *>(sequences_struct.data.data());
                
                auto current_index = sequence_index;
                auto remaining_sequences = sequence_count;
                
                while(remaining_sequences > 0) {
                    auto &sequence = sequences[current_index];
                    auto sprites_offset = sequences_struct.resolve_pointer(&sequence.sprites.pointer);

                    if(sprites_offset.has_value()) {
                        auto *sprites = reinterpret_cast<BitmapGroupSprite::struct_little *>(workload.structs[*sprites_offset].data.data());

                        // We'll need to iterate through all of the sprites
                        std::size_t sprite_count = sequence.sprites.count;
                        for(std::size_t i = 0; i < sprite_count; i++) {
                            auto &sprite = sprites[i];

                            // Get yer values here. Get 'em while they're hot.
                            float width_bitmap = 1.0F / std::fabs(sprite.right - sprite.left) / bitmap_dimensions[sprite.bitmap_index].first;
                            float height_bitmap = 1.0F / std::fabs(sprite.bottom - sprite.top) / bitmap_dimensions[sprite.bitmap_index].second;

                            // There!
                            pixel_size = std::min(pixel_size, std::min(height_bitmap, width_bitmap));
                            
                            if(bitmap_dimensions[sprite.bitmap_index].first != bitmap_dimensions[sprite.bitmap_index].second) {
                                warn = true;
                            }
                        }
                    }
                    
                    // Stop?
                    current_index++;
                    remaining_sequences--;
                    if(current_index == bitmap_sequence_count) {
                        break;
                    }
                }
            }
        }
        return pixel_size;
    }

    void Particle::postprocess_hek_data() {
        this->contact_deterioration = 0.0F;
        
        if(this->radius_animation.from == 0.0F) {
            this->radius_animation.from = 1.0F;
        }
        if(this->radius_animation.to == 0.0F) {
            this->radius_animation.to = 1.0F;
        }
    }
    
    static void complain_about_non_square_sheets(BuildWorkload &workload, std::size_t tag_index, const BuildWorkload::BuildWorkloadTag &tag) {
        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Bitmap %s.%s uses non-square sprite sheets. The particle will be distorted.", File::halo_path_to_preferred_path(tag.path).c_str(), HEK::tag_fourcc_to_extension(tag.tag_fourcc));
    }
    
    void Particle::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        bool warn;
        
        auto &particle = *reinterpret_cast<struct_little *>(workload.structs[struct_index].data.data() + offset);
        this->sprite_size = get_bitmap_tag_pixel_size(workload, this->bitmap.tag_id.index, this->first_sequence_index, static_cast<std::size_t>(this->initial_sequence_count) + static_cast<std::size_t>(this->looping_sequence_count) + static_cast<std::size_t>(this->final_sequence_count), warn);
        particle.sprite_size = this->sprite_size;
        particle.make_it_actually_work = 1;
        
        if(warn) {
            complain_about_non_square_sheets(workload, tag_index, workload.tags[this->bitmap.tag_id.index]);
        }
    }
    void WeatherParticleSystemParticleType::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        bool warn;
        auto &particle = *reinterpret_cast<struct_little *>(workload.structs[struct_index].data.data() + offset);
        this->sprite_size = get_bitmap_tag_pixel_size(workload, this->sprite_bitmap.tag_id.index, 0, 1, warn);
        particle.sprite_size = this->sprite_size;
        particle.not_broken = 1;
        
        if(warn) {
            complain_about_non_square_sheets(workload, tag_index, workload.tags[this->sprite_bitmap.tag_id.index]);
        }
    }
    void ParticleSystemTypeParticleState::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        bool warn;
        get_bitmap_tag_pixel_size(workload, this->bitmaps.tag_id.index, this->sequence_index, 1, warn);
        
        if(warn) {
            complain_about_non_square_sheets(workload, tag_index, workload.tags[this->bitmaps.tag_id.index]);
        }
    }
    void ParticleSystemTypeParticleState::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->unknown_int = 1;
    }
}
