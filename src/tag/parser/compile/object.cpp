// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/tag/parser/compile/shader.hpp>
#include <invader/tag/parser/compile/object.hpp>

#include "hud_interface.hpp"

namespace Invader::Parser {
    
    
    template <typename T> static void fix_render_bounding_radius(T &tag) {
        tag.render_bounding_radius = tag.render_bounding_radius < tag.bounding_radius ? tag.bounding_radius : tag.render_bounding_radius;
    }

    template <typename T> static void compile_object(T &tag, BuildWorkload &workload, std::size_t tag_index) {
        tag.scales_change_colors = 0;
        for(auto &c : tag.change_colors) {
            if(c.scale_by != HEK::FunctionScaleBy::FUNCTION_SCALE_BY_NONE) {
                tag.scales_change_colors = 1;
                break;
            }
        }
        fix_render_bounding_radius(tag);

        // Animate what?
        if(tag.model.path.size() == 0 && tag.animation_graph.path.size() != 0) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_FATAL_ERROR, "Object tag has an animation graph but no model tag", tag_index);
            throw InvalidTagDataException();
        }
    }

    void ObjectChangeColors::postprocess_hek_data() {
        // Get permutation count
        std::size_t permutation_count = this->permutations.size();
        if(permutation_count == 0) {
            return;
        }

        // Get the total
        double total = 0.0;
        for(auto &p : this->permutations) {
            total += p.weight;
        }

        // Default everything to 1/total if the total is 0
        if(total == 0.0) {
            total = static_cast<double>(this->permutations.size());
            for(auto &p : this->permutations) {
                p.weight = static_cast<float>(1.0 / total);
            }
        }
        // Normalize everything to 0-1 otherwise
        else {
            for(auto &p : this->permutations) {
                p.weight = p.weight / total;
            }
        }
    }

    void ObjectChangeColors::post_cache_deformat() {
        // Get permutation count
        std::size_t permutation_count = this->permutations.size();
        if(permutation_count == 0) {
            return;
        }

        // Get back something now
        double progress = 0.0;
        for(auto &p : this->permutations) {
            double difference = p.weight - progress;
            progress = p.weight;
            p.weight = static_cast<float>(difference);
        }
    }

    void ObjectChangeColors::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        // Get permutation count
        std::size_t permutation_count = this->permutations.size();
        if(permutation_count == 0) {
            return;
        }

        // Get the total
        double total = 0.0;
        for(auto &p : this->permutations) {
            total += p.weight;
        }

        // If the total is zero, even them out. This will likely already be done in postprocess_hek_data() anyway
        if(total == 0.0) {
            total = static_cast<double>(this->permutations.size());
            for(auto &p : this->permutations) {
                p.weight = static_cast<float>(1.0 / total);
            }
        }

        // Now write this stuff
        double progress = 0.0;
        for(auto &p : this->permutations) {
            p.weight = p.weight / total + progress;
            progress = p.weight;
        }

        // Set the last one to 1.0
        this->permutations[permutation_count - 1].weight = 1.0F;
    }

    void Biped::postprocess_hek_data() {
        fix_render_bounding_radius(*this);
    }

    void Vehicle::postprocess_hek_data() {
        fix_render_bounding_radius(*this);
    }

    void Weapon::postprocess_hek_data() {
        fix_render_bounding_radius(*this);
    }

    void Equipment::postprocess_hek_data() {
        fix_render_bounding_radius(*this);
    }

    void Garbage::postprocess_hek_data() {
        fix_render_bounding_radius(*this);
    }

    void DeviceControl::postprocess_hek_data() {
        fix_render_bounding_radius(*this);
    }

    void DeviceLightFixture::postprocess_hek_data() {
        fix_render_bounding_radius(*this);
    }

    void DeviceMachine::postprocess_hek_data() {
        fix_render_bounding_radius(*this);
    }

    void Scenery::postprocess_hek_data() {
        fix_render_bounding_radius(*this);
    }

    void Placeholder::postprocess_hek_data() {
        fix_render_bounding_radius(*this);
    }

    void Projectile::postprocess_hek_data() {
        fix_render_bounding_radius(*this);
    }

    template <typename T> void compile_unit(T &tag) {
        tag.soft_ping_interrupt_ticks = static_cast<std::int16_t>(tag.soft_ping_interrupt_time * TICK_RATE);
        tag.hard_ping_interrupt_ticks = static_cast<std::int16_t>(tag.hard_ping_interrupt_time * TICK_RATE);
    }

    void Biped::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_BIPED;
        compile_object(*this, workload, tag_index);
        compile_unit(*this);
        this->cosine_stationary_turning_threshold = std::cos(this->stationary_turning_threshold);
        
        if(this->crouch_transition_time == 0.0F) {
            this->crouch_camera_velocity = 1.0F;
        }
        else {
            this->crouch_camera_velocity = TICK_RATE_RECIPROCOL / this->crouch_transition_time;
        }
        
        this->cosine_maximum_slope_angle = static_cast<float>(std::cos(this->maximum_slope_angle));
        this->negative_sine_downhill_falloff_angle = static_cast<float>(-std::sin(this->downhill_falloff_angle));
        this->negative_sine_downhill_cutoff_angle = static_cast<float>(-std::sin(this->downhill_cutoff_angle));
        this->sine_uphill_falloff_angle = static_cast<float>(std::sin(this->uphill_falloff_angle));
        this->sine_uphill_cutoff_angle = static_cast<float>(std::sin(this->uphill_cutoff_angle));
    }
    void Vehicle::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_VEHICLE;
        compile_object(*this, workload, tag_index);
        compile_unit(*this);
    }
    void Weapon::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_WEAPON;
        compile_object(*this, workload, tag_index);

        auto engine_target = workload.get_build_parameters()->details.build_cache_file_engine;

        // Jason jones autoaim for the rocket warthog
        if(workload.building_stock_map && (workload.tags[tag_index].path == "vehicles\\rwarthog\\rwarthog_gun")) {
            bool native_or_custom_edition = engine_target == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION || engine_target == HEK::CacheFileEngine::CACHE_FILE_NATIVE;
            float new_autoaim_angle = native_or_custom_edition ? DEGREES_TO_RADIANS(6.0F) : DEGREES_TO_RADIANS(1.0F);
            float new_deviation_angle = native_or_custom_edition ? DEGREES_TO_RADIANS(12.0F) : DEGREES_TO_RADIANS(1.0F);

            if(new_autoaim_angle != this->autoaim_angle || new_deviation_angle != this->deviation_angle) {
                workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING_PEDANTIC, "Autoaim angles were changed due to building a stock scenario", tag_index);

                this->deviation_angle = new_deviation_angle;
                this->autoaim_angle = new_autoaim_angle;
            }
        }
    }
    void Equipment::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_EQUIPMENT;
        compile_object(*this, workload, tag_index);
    }
    void Garbage::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_GARBAGE;
        compile_object(*this, workload, tag_index);
    }
    void Projectile::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_PROJECTILE;
        compile_object(*this, workload, tag_index);
        this->minimum_velocity *= TICK_RATE_RECIPROCOL;
        this->initial_velocity *= TICK_RATE_RECIPROCOL;
        this->final_velocity *= TICK_RATE_RECIPROCOL;
    }
    void ProjectileMaterialResponse::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->potential_and.from *= TICK_RATE_RECIPROCOL;
        this->potential_and.to *= TICK_RATE_RECIPROCOL;
    }
    void Scenery::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_SCENERY;
        compile_object(*this, workload, tag_index);
    }
    void Placeholder::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_PLACEHOLDER;
        compile_object(*this, workload, tag_index);
    }
    void SoundScenery::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_SOUND_SCENERY;
        compile_object(*this, workload, tag_index);
    }

    void DeviceMachine::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_DEVICE_MACHINE;
        compile_object(*this, workload, tag_index);
        this->door_open_time_ticks = static_cast<std::uint32_t>(this->door_open_time * TICK_RATE);
    }
    void DeviceControl::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_DEVICE_CONTROL;
        compile_object(*this, workload, tag_index);
    }
    void DeviceLightFixture::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->object_type = HEK::ObjectType::OBJECT_TYPE_DEVICE_LIGHT_FIXTURE;
        compile_object(*this, workload, tag_index);
    }

    void ObjectFunction::pre_compile(BuildWorkload &, std::size_t, std::size_t, std::size_t) {
        this->inverse_bounds = 1.0f / (this->bounds.to - this->bounds.from);
        if(this->step_count > 1) {
            this->inverse_step = 1.0f / (this->step_count - 1);
        }
        this->inverse_period = 1.0f / this->period;
        if(this->sawtooth_count > 1) {
            this->inverse_sawtooth = 1.0f / (this->sawtooth_count - 1);
        }
    }

    void WeaponTrigger::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t offset) {
        this->illumination_recovery_rate = this->illumination_recovery_time <= 0.0F ? 1.0F : (TICK_RATE_RECIPROCOL / this->illumination_recovery_time);
        this->ejection_port_recovery_rate = this->ejection_port_recovery_time <= 0.0F ? 1.0F : (TICK_RATE_RECIPROCOL / this->ejection_port_recovery_time);

        this->firing_acceleration_rate = this->acceleration_time <= 0.0F ? 1.0F : (TICK_RATE_RECIPROCOL / this->acceleration_time);
        this->firing_deceleration_rate = this->deceleration_time <= 0.0F ? 1.0F : (TICK_RATE_RECIPROCOL / this->deceleration_time);

        this->error_acceleration_rate = this->error_acceleration_time <= 0.0F ? 1.0F : (TICK_RATE_RECIPROCOL / this->error_acceleration_time);
        this->error_deceleration_rate = this->error_deceleration_time <= 0.0F ? 1.0F : (TICK_RATE_RECIPROCOL / this->error_deceleration_time);
        
        this->flags |= 0x10000;

        // Jason Jones the accuracy of the weapon
        if(offset == 0 && workload.cache_file_type.has_value() && workload.cache_file_type.value() == HEK::CacheFileType::SCENARIO_TYPE_SINGLEPLAYER) {
            auto &tag = workload.tags[tag_index];
            if(tag.path == "weapons\\pistol\\pistol") {
                this->minimum_error = DEGREES_TO_RADIANS(0.2F);
                this->error_angle.from = DEGREES_TO_RADIANS(0.2F);
                this->error_angle.to = DEGREES_TO_RADIANS(0.4F);
            }
            else if(tag.path == "weapons\\plasma rifle\\plasma rifle") {
                this->error_angle.from = DEGREES_TO_RADIANS(0.25F);
                this->error_angle.to = DEGREES_TO_RADIANS(2.5F);
            }
        }

        // Warn if we're trying to fire a negative number of projectiles per shot
        if(this->projectiles_per_shot < 0) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Trigger #%zu has a negative number of projectiles per shot, thus no projectiles will spawn. Set it to 0 to silence this warning.", offset / sizeof(struct_little));
        }

        // Warn if we have rounds per shot set but no magazine
        if(this->rounds_per_shot != 0 && this->magazine == NULL_INDEX) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Trigger #%zu has rounds per shot set with no magazine, thus it will not actually use any rounds per shot. Set it to 0 or set a magazine to silence this warning.", offset / sizeof(struct_little));
        }

        // Warn if the weapon doesn't fire automatically but we have projectiles per contrail set or we have a meme value.
        if(this->projectiles_between_contrails < 0) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Trigger #%zu has a negative number of projectiles between contrails, thus all projectiles will have contrails. Set it to 0 to silence this warning.", offset / sizeof(struct_little));
        }
        else if(this->projectiles_between_contrails > 0 && (this->flags & HEK::WeaponTriggerFlagsFlag::WEAPON_TRIGGER_FLAGS_FLAG_DOES_NOT_REPEAT_AUTOMATICALLY)) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Trigger #%zu has a nonzero number of projectiles between contrails, but this is ignored because the trigger is set to not fire automatically. Set it to 0 to silence this warning.", offset / sizeof(struct_little));
        }
    }

    void recursively_get_all_predicted_resources_from_struct(const BuildWorkload &workload, std::size_t struct_index, std::vector<std::size_t> &resources, bool ignore_shader_resources) {
        if(workload.disable_recursion) {
            return;
        }

        auto &s = workload.structs[struct_index];
        for(auto &d : s.dependencies) {
            std::size_t tag_index = d.tag_index;
            auto &dt = workload.tags[tag_index];
            switch(dt.tag_fourcc) {
                case TagFourCC::TAG_FOURCC_BITMAP:
                case TagFourCC::TAG_FOURCC_SOUND:
                    if(!ignore_shader_resources) {
                        resources.push_back(tag_index);
                    }
                    break;
                case TagFourCC::TAG_FOURCC_SHADER_ENVIRONMENT:
                case TagFourCC::TAG_FOURCC_SHADER_MODEL:
                case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO:
                case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO_EXTENDED:
                case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_GENERIC:
                case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_GLASS:
                case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_METER:
                case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_PLASMA:
                case TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_WATER:
                    if(ignore_shader_resources) {
                        break;
                    }
                    // fallthrough
                case TagFourCC::TAG_FOURCC_GBXMODEL:
                    recursively_get_all_predicted_resources_from_struct(workload, *dt.base_struct, resources, false);
                    break;
                default:
                    continue;
            }
        }
        for(auto &p : s.pointers) {
            recursively_get_all_predicted_resources_from_struct(workload, p.struct_index, resources, ignore_shader_resources);
        }
    }

    // Go through each tag this depends on that can have predicted resources. This is to preload assets when the object spawns to reduce hitching. Using maps in RAM makes this basically pointless, though.
    static void calculate_object_predicted_resources(BuildWorkload &workload, std::size_t struct_index) {
        std::vector<std::size_t> resources;
        recursively_get_all_predicted_resources_from_struct(workload, struct_index, resources, true);
        auto &s = workload.structs[struct_index];

        for(std::size_t a = 0; a < resources.size(); a++) {
            for(std::size_t b = a + 1; b < resources.size(); b++) {
                if(resources[a] == resources[b]) {
                    resources.erase(resources.begin() + b);
                    b--;
                }
            }
        }

        std::size_t resources_count = resources.size();
        if(resources_count > 0) {
            auto &predicted_resource_pointer = s.pointers.emplace_back();
            predicted_resource_pointer.struct_index = workload.structs.size();

            auto &object = *reinterpret_cast<Parser::Object::struct_little *>(s.data.data());
            predicted_resource_pointer.offset = reinterpret_cast<const std::byte *>(&object.predicted_resources.pointer) - reinterpret_cast<const std::byte *>(&object);
            object.predicted_resources.count = static_cast<std::uint32_t>(resources_count);
            std::vector<HEK::PredictedResource<HEK::LittleEndian>> predicted_resources;

            auto &prs = workload.structs.emplace_back();
            predicted_resources.reserve(resources_count);
            prs.dependencies.reserve(resources_count);
            for(std::size_t r : resources) {
                auto &resource = predicted_resources.emplace_back();
                auto &resource_tag = workload.tags[r];
                resource.type = resource_tag.tag_fourcc == TagFourCC::TAG_FOURCC_BITMAP ? HEK::PredictedResourceType::PREDICTED_RESOURCE_TYPE_BITMAP : HEK::PredictedResourceType::PREDICTED_RESOURCE_TYPE_SOUND;
                resource.tag = HEK::TagID { static_cast<std::uint32_t>(r) };
                resource.resource_index = 0;
                auto &resource_dep = prs.dependencies.emplace_back();
                resource_dep.tag_id_only = true;
                resource_dep.tag_index = r;
                resource_dep.offset = reinterpret_cast<const std::byte *>(&resource.tag) - reinterpret_cast<const std::byte *>(predicted_resources.data());
            }
            prs.data.insert(prs.data.begin(), reinterpret_cast<const std::byte *>(predicted_resources.data()), reinterpret_cast<const std::byte *>(predicted_resources.data() + resources_count));
        }
    }

    static void validate_model_animation_checksum(BuildWorkload &workload, std::size_t tag_index, HEK::TagID model, HEK::TagID animations) {
        if(model.is_null() || animations.is_null() || workload.disable_recursion) {
            return;
        }

        // Read the checksum
        auto &model_tag = workload.tags[model.index];
        auto model_checksum = reinterpret_cast<Model::struct_little *>(workload.structs[*model_tag.base_struct].data.data())->node_list_checksum.read();

        // And, yes, reading it as a model is OK since they're in the same offset, so we don't need to do any extra logic here
        static_assert(offsetof(Model::struct_little, node_list_checksum) == offsetof(GBXModel::struct_little, node_list_checksum));

        // If it's 0, don't bother checking. This is a complete hack, but the official tools do this. And if you're lucky and make a tag that just so happens to have a checksum of 0, you win a free warning!
        if(model_checksum == 0) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "%s.%s has a node list checksum of 0, so its checksum will not be checked", File::halo_path_to_preferred_path(model_tag.path).c_str(), HEK::tag_fourcc_to_extension(model_tag.tag_fourcc));
            return;
        }

        // Now iterate through animations
        auto &animation_tag = workload.tags[animations.index];
        auto &animation_struct = workload.structs[*animation_tag.base_struct];
        auto &animation_struct_data = *reinterpret_cast<Parser::ModelAnimations::struct_little *>(animation_struct.data.data());
        auto animation_count = animation_struct_data.animations.count.read();
        if(animation_count > 0) {
            auto *animations = reinterpret_cast<Parser::ModelAnimationsAnimation::struct_little *>(workload.structs[*animation_struct.resolve_pointer(&animation_struct_data.animations.pointer)].data.data());
            for(std::size_t a = 0; a < animation_count; a++) {
                auto animation_checksum = animations[a].node_list_checksum.read();

                // Same hack as before. And again, if your lucky lotto number is 0, congratulations!
                if(animation_checksum == 0) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "%s.%s animation #%zu has a node list checksum of 0, so its checksum will not be checked", File::halo_path_to_preferred_path(animation_tag.path).c_str(), HEK::tag_fourcc_to_extension(animation_tag.tag_fourcc), a);
                    continue;
                }

                // Check it
                if(model_checksum != animation_checksum) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "%s.%s and %s.%s animation #%zu node list checksums are mismatched", File::halo_path_to_preferred_path(model_tag.path).c_str(),
                                                                                                                                                          HEK::tag_fourcc_to_extension(model_tag.tag_fourcc),
                                                                                                                                                          File::halo_path_to_preferred_path(animation_tag.path).c_str(),
                                                                                                                                                          HEK::tag_fourcc_to_extension(animation_tag.tag_fourcc),
                                                                                                                                                          a);
                    throw InvalidTagDataException();
                }
            }
        }
    }

    static void validate_collision_model_regions(BuildWorkload &workload, std::size_t tag_index, HEK::TagID model, HEK::TagID collision_model) {
        if(model.is_null() || collision_model.is_null() || workload.disable_recursion) {
            return;
        }

        // Read the thing
        auto &model_tag = workload.tags[model.index];
        auto &collision_tag = workload.tags[collision_model.index];

        auto &model_struct = *reinterpret_cast<Parser::Model::struct_little *>(workload.structs[*model_tag.base_struct].data.data());
        auto &collision_struct = *reinterpret_cast<Parser::ModelCollisionGeometry::struct_little *>(workload.structs[*collision_tag.base_struct].data.data());

        // And, as always, reading it as a model is OK since they're in the same offset, so we don't need to do any extra logic here
        static_assert(offsetof(Model::struct_little, regions) == offsetof(GBXModel::struct_little, regions));

        // Error if they are different
        if(model_struct.regions.count.read() != collision_struct.regions.count.read()) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "%s.%s and %s.%s's region counts are mismatched", File::halo_path_to_preferred_path(model_tag.path).c_str(),
                                                                                                                               HEK::tag_fourcc_to_extension(model_tag.tag_fourcc),
                                                                                                                               File::halo_path_to_preferred_path(collision_tag.path).c_str(),
                                                                                                                               HEK::tag_fourcc_to_extension(collision_tag.tag_fourcc));
            throw InvalidTagDataException();
        }
    }

    static void set_pathfinding_spheres(BuildWorkload &workload, std::size_t struct_index, std::optional<float> collision_radius = std::nullopt) {
        // If recursion is disabled, do not do this
        if(workload.disable_recursion) {
            return;
        }

        // Get our object
        auto &object_struct = workload.structs[struct_index];
        auto &object_data = *reinterpret_cast<Object::struct_little *>(object_struct.data.data());

        // Do we have a collision model?
        auto collision_id = object_data.collision_model.tag_id.read();
        if(collision_id.is_null()) {
            return; // nope
        }

        // Get what we need
        auto &collision_struct = workload.structs[*workload.tags[collision_id.index].base_struct];
        auto *collision_data_ptr = collision_struct.data.data();
        auto &collision_data = *reinterpret_cast<ModelCollisionGeometry::struct_little *>(collision_data_ptr);

        // Do we already have pathfinding spheres?
        if(collision_data.pathfinding_spheres.count.read() != 0) {
            return; // yep
        }

        // Start finding the thing, then
        float x = 0.0F, y = 0.0F, z = 0.0F;
        HEK::Index node_index;

        // If we don't have a value set, set one.
        float sphere_radius;
        if(!collision_radius.has_value()) {
            // Pathfinding sphere radius:
            //     2^(log4(bounding_radius)) * 3 / 4   if bounding_radius > 1
            //     bounding_radius * 3 / 4             if bounding_radius > 0 and bounding_radius <= 1
            float v = object_data.bounding_radius.read();
            if(v > 1.0) {
                v = std::pow(2.0, std::log(v) / std::log(4));
            }
            else if(v <= 0.0) {
                return; // no pathfinding sphere
            }
            sphere_radius = static_cast<float>(v * 3.0 / 4.0);
            node_index = 0;

            // Set the offset to the bounding offset of the object
            x = object_data.bounding_offset.x.read();
            y = object_data.bounding_offset.y.read();
            z = object_data.bounding_offset.z.read();
        }
        else {
            // Bounding offset is not used with bipeds when making pathfinding spheres; the z value and sphere radius is set to the bounding radius.
            z = collision_radius.value();
            sphere_radius = z;
            node_index = NULL_INDEX;
            if(z <= 0.0) {
                return; // no pathfinding sphere
            }
        }

        // Let's make that pathfinding sphere
        auto &pathfinding_ptr = collision_struct.pointers.emplace_back();
        pathfinding_ptr.struct_index = workload.structs.size();
        pathfinding_ptr.offset = reinterpret_cast<std::byte *>(&collision_data.pathfinding_spheres.pointer) - collision_data_ptr;
        collision_data.pathfinding_spheres.count = 1;

        auto &pathfinding_struct = workload.structs.emplace_back();
        pathfinding_struct.data.resize(sizeof(ModelCollisionGeometrySphere::struct_little));
        auto &sphere = *reinterpret_cast<ModelCollisionGeometrySphere::struct_little *>(pathfinding_struct.data.data());
        sphere.radius = sphere_radius;
        sphere.center.x = x;
        sphere.center.y = y;
        sphere.center.z = z;
        sphere.node = node_index;
    }

    void Biped::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        if(workload.disable_recursion) {
            return; // if recursion is disabled, doing any of this will be a meme
        }

        auto &struct_val = *reinterpret_cast<struct_little *>(workload.structs[struct_index].data.data() + offset);
        auto &model_id = this->model.tag_id;

        struct_val.head_model_node_index = NULL_INDEX;
        struct_val.pelvis_model_node_index = NULL_INDEX;

        if(struct_val.animation_graph.tag_id.read().is_null()) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "Biped has no animation graph, so the biped will not spawn", tag_index);
        }
        else {
            auto &model_tag = workload.tags[model_id.index];
            auto &model_tag_header = workload.structs[*model_tag.base_struct];
            auto &model_tag_header_struct = *reinterpret_cast<GBXModel::struct_little *>(model_tag_header.data.data());
            std::size_t node_count = model_tag_header_struct.nodes.count.read();
            if(node_count) {
                auto *nodes = reinterpret_cast<ModelNode::struct_little *>(workload.structs[*model_tag_header.resolve_pointer(&model_tag_header_struct.nodes.pointer)].data.data());
                for(std::size_t n = 0; n < node_count; n++) {
                    auto &node = nodes[n];
                    if(std::strncmp(node.name.string, "bip01 head", sizeof(node.name.string) - 1) == 0) {
                        struct_val.head_model_node_index = static_cast<HEK::Index>(n);
                    }
                    if(std::strncmp(node.name.string, "bip01 pelvis", sizeof(node.name.string) - 1) == 0) {
                        struct_val.pelvis_model_node_index = static_cast<HEK::Index>(n);
                    }
                }
            }
        }

        calculate_object_predicted_resources(workload, struct_index);
        set_pathfinding_spheres(workload, struct_index, struct_val.collision_radius);
        validate_model_animation_checksum(workload, tag_index, this->model.tag_id, this->animation_graph.tag_id);
        validate_collision_model_regions(workload, tag_index, this->model.tag_id, this->collision_model.tag_id);
    }
    void Vehicle::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset) {
        auto &struct_val = *reinterpret_cast<struct_little *>(workload.structs[struct_index].data.data() + offset);

        if(struct_val.animation_graph.tag_id.read().is_null()) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "Vehicle has no animation graph, so the vehicle will not spawn", tag_index);
        }

        calculate_object_predicted_resources(workload, struct_index);
        set_pathfinding_spheres(workload, struct_index);
        validate_model_animation_checksum(workload, tag_index, this->model.tag_id, this->animation_graph.tag_id);
        validate_collision_model_regions(workload, tag_index, this->model.tag_id, this->collision_model.tag_id);
    }
    void Weapon::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        if(workload.disable_recursion) {
            return;
        }

        // Make sure zoom levels aren't too high for the HUD interface
        if(this->zoom_levels && !this->hud_interface.tag_id.is_null()) {
            auto &weapon_hud_interface_tag = workload.tags[this->hud_interface.tag_id.index];
            auto &weapon_hud_interface_struct = workload.structs[*weapon_hud_interface_tag.base_struct];
            auto &weapon_hud_interface = *reinterpret_cast<const WeaponHUDInterface::struct_little *>(weapon_hud_interface_struct.data.data());
            std::size_t crosshair_count = weapon_hud_interface.crosshairs.count;
            if(crosshair_count) {
                auto &crosshairs_struct = workload.structs[weapon_hud_interface_struct.resolve_pointer(&weapon_hud_interface.crosshairs.pointer).value()];
                auto *crosshairs = reinterpret_cast<const WeaponHUDInterfaceCrosshair::struct_little *>(crosshairs_struct.data.data());
                for(std::size_t c = 0; c < crosshair_count; c++) {
                    auto &crosshair = crosshairs[c];
                    if(crosshair.crosshair_type != HEK::WeaponHUDInterfaceCrosshairType::WEAPON_HUD_INTERFACE_CROSSHAIR_TYPE_ZOOM_OVERLAY) {
                        continue;
                    }

                    std::size_t overlay_count = crosshair.crosshair_overlays.count;
                    if(overlay_count) {
                        const BitmapGroupSequence::struct_little *sequences;
                        std::size_t sequence_count;
                        char bitmap_tag_path[512];
                        HEK::BitmapType bitmap_type;
                        get_sequence_data(workload, crosshair.crosshair_bitmap.tag_id, sequence_count, sequences, bitmap_tag_path, sizeof(bitmap_tag_path), bitmap_type);
                        auto &crosshair_overlays_struct = workload.structs[*crosshairs_struct.resolve_pointer(&crosshair.crosshair_overlays.pointer)];
                        auto *crosshair_overlays = reinterpret_cast<const WeaponHUDInterfaceCrosshairOverlay::struct_little *>(crosshair_overlays_struct.data.data());

                        // Make sure stuff is there
                        for(std::size_t o = 0; o < overlay_count; o++) {
                            auto &overlay = crosshair_overlays[o];
                            if(overlay.sequence_index == NULL_INDEX) {
                                continue;
                            }
                            // if this is false, the HUD interface tag will error on building anyway - no need to make multiple errors that are the same thing if we can help it
                            else if(overlay.sequence_index < sequence_count) {
                                auto &sequence = sequences[overlay.sequence_index];
                                bool not_a_sprite = overlay.flags.read() & HEK::WeaponHUDInterfaceCrosshairOverlayFlagsFlag::WEAPON_HUD_INTERFACE_CROSSHAIR_OVERLAY_FLAGS_FLAG_NOT_A_SPRITE;
                                std::size_t max_zoom_levels = not_a_sprite ? sequence.bitmap_count.read() : sequence.sprites.count.read();
                                if(this->zoom_levels < 0 || static_cast<std::size_t>(this->zoom_levels) > max_zoom_levels) {
                                    const char *noun;
                                    if(not_a_sprite) {
                                        noun = "bitmap";
                                    }
                                    else {
                                        noun = "sprite";
                                    }
                                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Weapon has %zu zoom level%s, but the sequence referenced in crosshair overlay #%zu of crosshair #%zu only has %zu %s%s", static_cast<std::size_t>(this->zoom_levels), this->zoom_levels == 1 ? "" : "s", o, c, max_zoom_levels, noun, max_zoom_levels == 1 ? "" : "s");
                                    throw InvalidTagDataException();
                                }
                            }
                        }
                    }
                }
            }
        }

        calculate_object_predicted_resources(workload, struct_index);
        validate_model_animation_checksum(workload, tag_index, this->model.tag_id, this->animation_graph.tag_id);
        validate_collision_model_regions(workload, tag_index, this->model.tag_id, this->collision_model.tag_id);

        // we do not check animations for first person because they do not correspond to one another
    }
    void Equipment::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        calculate_object_predicted_resources(workload, struct_index);
        validate_model_animation_checksum(workload, tag_index, this->model.tag_id, this->animation_graph.tag_id);
        validate_collision_model_regions(workload, tag_index, this->model.tag_id, this->collision_model.tag_id);
    }
    void Garbage::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        calculate_object_predicted_resources(workload, struct_index);
        validate_model_animation_checksum(workload, tag_index, this->model.tag_id, this->animation_graph.tag_id);
        validate_collision_model_regions(workload, tag_index, this->model.tag_id, this->collision_model.tag_id);
    }
    void Projectile::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        calculate_object_predicted_resources(workload, struct_index);
        validate_model_animation_checksum(workload, tag_index, this->model.tag_id, this->animation_graph.tag_id);
        validate_collision_model_regions(workload, tag_index, this->model.tag_id, this->collision_model.tag_id);
    }
    void Scenery::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        calculate_object_predicted_resources(workload, struct_index);
        set_pathfinding_spheres(workload, struct_index);
        validate_model_animation_checksum(workload, tag_index, this->model.tag_id, this->animation_graph.tag_id);
        validate_collision_model_regions(workload, tag_index, this->model.tag_id, this->collision_model.tag_id);
    }
    void Placeholder::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        calculate_object_predicted_resources(workload, struct_index);
        validate_model_animation_checksum(workload, tag_index, this->model.tag_id, this->animation_graph.tag_id);
        validate_collision_model_regions(workload, tag_index, this->model.tag_id, this->collision_model.tag_id);
    }
    void SoundScenery::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t) {
        calculate_object_predicted_resources(workload, struct_index);
        validate_model_animation_checksum(workload, tag_index, this->model.tag_id, this->animation_graph.tag_id);
        validate_collision_model_regions(workload, tag_index, this->model.tag_id, this->collision_model.tag_id);
    }

    void device_post_compile(BuildWorkload &workload, ::size_t struct_index, std::size_t struct_offset) {
        auto &device = *reinterpret_cast<Device::struct_little *>(workload.structs[struct_index].data.data() + struct_offset);

        auto set_inverse = [](auto &from, auto &to) {
            if(from.read() != 0) {
                to = 1.0F / (TICK_RATE * from);
            }
        };

        set_inverse(device.power_transition_time, device.inverse_power_transition_time);
        set_inverse(device.power_acceleration_time, device.inverse_power_acceleration_time);
        set_inverse(device.position_transition_time, device.inverse_position_transition_time);
        set_inverse(device.position_acceleration_time, device.inverse_position_acceleration_time);
        set_inverse(device.depowered_position_transition_time, device.inverse_depowered_position_transition_time);
        set_inverse(device.depowered_position_acceleration_time, device.inverse_depowered_position_acceleration_time);

        device.delay_time_ticks = TICK_RATE * device.delay_time;
    }

    void DeviceMachine::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t struct_offset) {
        device_post_compile(workload, struct_index, struct_offset);
        calculate_object_predicted_resources(workload, struct_index);
        set_pathfinding_spheres(workload, struct_index);
        validate_model_animation_checksum(workload, tag_index, this->model.tag_id, this->animation_graph.tag_id);
        validate_collision_model_regions(workload, tag_index, this->model.tag_id, this->collision_model.tag_id);
    }
    void DeviceControl::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t struct_offset) {
        device_post_compile(workload, struct_index, struct_offset);
        calculate_object_predicted_resources(workload, struct_index);
        validate_model_animation_checksum(workload, tag_index, this->model.tag_id, this->animation_graph.tag_id);
        validate_collision_model_regions(workload, tag_index, this->model.tag_id, this->collision_model.tag_id);
    }
    void DeviceLightFixture::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t struct_offset) {
        device_post_compile(workload, struct_index, struct_offset);
        calculate_object_predicted_resources(workload, struct_index);
        validate_model_animation_checksum(workload, tag_index, this->model.tag_id, this->animation_graph.tag_id);
        validate_collision_model_regions(workload, tag_index, this->model.tag_id, this->collision_model.tag_id);
    }
    
    template<typename From, typename To> static void convert_object(const From &from, To &to) {
        to.flags = from.flags;
        to.bounding_radius = from.bounding_radius;
        to.bounding_offset = from.bounding_offset;
        to.origin_offset = from.origin_offset;
        to.acceleration_scale = from.acceleration_scale;
        to.scales_change_colors = from.scales_change_colors;
        to.model = from.model;
        to.animation_graph = from.animation_graph;
        to.collision_model = from.collision_model;
        to.physics = from.physics;
        to.modifier_shader = from.modifier_shader;
        to.creation_effect = from.creation_effect;
        to.render_bounding_radius = from.render_bounding_radius;
        to.a_in = from.a_in;
        to.b_in = from.b_in;
        to.c_in = from.c_in;
        to.d_in = from.d_in;
        to.hud_text_message_index = from.hud_text_message_index;
        to.forced_shader_permutation_index = from.forced_shader_permutation_index;
        to.attachments = from.attachments;
        to.widgets = from.widgets;
        to.functions = from.functions;
        to.change_colors = from.change_colors;
    }
    
    template<typename From, typename To> static void convert_unit(const From &from, To &to) {
        convert_object(from, to);
        to.unit_flags = from.unit_flags;
        to.default_team = from.default_team;
        to.constant_sound_volume = from.constant_sound_volume;
        to.rider_damage_fraction = from.rider_damage_fraction;
        to.integrated_light_toggle = from.integrated_light_toggle;
        to.unit_a_in = from.unit_a_in;
        to.unit_b_in = from.unit_b_in;
        to.unit_c_in = from.unit_c_in;
        to.unit_d_in = from.unit_d_in;
        to.camera_field_of_view = from.camera_field_of_view;
        to.camera_stiffness = from.camera_stiffness;
        to.camera_marker_name = from.camera_marker_name;
        to.camera_submerged_marker_name = from.camera_submerged_marker_name;
        to.pitch_auto_level = from.pitch_auto_level;
        to.pitch_range = from.pitch_range;
        to.camera_tracks = from.camera_tracks;
        to.seat_acceleration_scale = from.seat_acceleration_scale;
        to.soft_ping_threshold = from.soft_ping_threshold;
        to.soft_ping_interrupt_time = from.soft_ping_interrupt_time;
        to.hard_ping_threshold = from.hard_ping_threshold;
        to.hard_ping_interrupt_time = from.hard_ping_interrupt_time;
        to.hard_death_threshold = from.hard_death_threshold;
        to.feign_death_threshold = from.feign_death_threshold;
        to.feign_death_time = from.feign_death_time;
        to.distance_of_evade_anim = from.distance_of_evade_anim;
        to.distance_of_dive_anim = from.distance_of_dive_anim;
        to.stunned_movement_threshold = from.stunned_movement_threshold;
        to.feign_death_chance = from.feign_death_chance;
        to.feign_repeat_chance = from.feign_repeat_chance;
        to.spawned_actor = from.spawned_actor;
        to.spawned_actor_count = from.spawned_actor_count;
        to.spawned_velocity = from.spawned_velocity;
        to.aiming_velocity_maximum = from.aiming_velocity_maximum;
        to.aiming_acceleration_maximum = from.aiming_acceleration_maximum;
        to.casual_aiming_modifier = from.casual_aiming_modifier;
        to.looking_velocity_maximum = from.looking_velocity_maximum;
        to.looking_acceleration_maximum = from.looking_acceleration_maximum;
        to.ai_vehicle_radius = from.ai_vehicle_radius;
        to.ai_danger_radius = from.ai_danger_radius;
        to.melee_damage = from.melee_damage;
        to.motion_sensor_blip_size = from.motion_sensor_blip_size;
        to.metagame_type = from.metagame_type;
        to.metagame_class = from.metagame_class;
        to.new_hud_interfaces = from.new_hud_interfaces;
        to.dialogue_variants = from.dialogue_variants;
        to.grenade_velocity = from.grenade_velocity;
        to.grenade_type = from.grenade_type;
        to.grenade_count = from.grenade_count;
        to.soft_ping_interrupt_ticks = from.soft_ping_interrupt_ticks;
        to.hard_ping_interrupt_ticks = from.hard_ping_interrupt_ticks;
        to.powered_seats = from.powered_seats;
        to.weapons = from.weapons;
        to.seats = from.seats;
    }
    
    void convert_object(const ParserStruct &from, ParserStruct &to) {
        auto convert_unit_maybe = [](auto *a, auto *b) -> bool {
            if(a && b) {
                convert_unit(*a, *b);
                return true;
            }
            else {
                return false;
            }
        };
        auto convert_object_maybe = [](auto *a, auto *b) -> bool {
            if(a && b) {
                convert_object(*a, *b);
                return true;
            }
            else {
                return false;
            }
        };
        
#define ATTEMPT_CONVERT_TYPE(FROM) (\
                                   convert_object_maybe(dynamic_cast<const FROM *>(&from), dynamic_cast<Parser::Biped *>(&to)) ||\
                                   convert_object_maybe(dynamic_cast<const FROM *>(&from), dynamic_cast<Parser::Vehicle *>(&to)) ||\
                                   convert_object_maybe(dynamic_cast<const FROM *>(&from), dynamic_cast<Parser::Weapon *>(&to)) ||\
                                   convert_object_maybe(dynamic_cast<const FROM *>(&from), dynamic_cast<Parser::Equipment *>(&to)) ||\
                                   convert_object_maybe(dynamic_cast<const FROM *>(&from), dynamic_cast<Parser::Garbage *>(&to)) ||\
                                   convert_object_maybe(dynamic_cast<const FROM *>(&from), dynamic_cast<Parser::Projectile *>(&to)) ||\
                                   convert_object_maybe(dynamic_cast<const FROM *>(&from), dynamic_cast<Parser::Scenery *>(&to)) ||\
                                   convert_object_maybe(dynamic_cast<const FROM *>(&from), dynamic_cast<Parser::DeviceMachine *>(&to)) ||\
                                   convert_object_maybe(dynamic_cast<const FROM *>(&from), dynamic_cast<Parser::DeviceControl *>(&to)) ||\
                                   convert_object_maybe(dynamic_cast<const FROM *>(&from), dynamic_cast<Parser::DeviceLightFixture *>(&to)) ||\
                                   convert_object_maybe(dynamic_cast<const FROM *>(&from), dynamic_cast<Parser::Placeholder *>(&to)) ||\
                                   convert_object_maybe(dynamic_cast<const FROM *>(&from), dynamic_cast<Parser::SoundScenery *>(&to))\
                                   )
        
        
        if(convert_unit_maybe(dynamic_cast<const Parser::Biped *>(&from), dynamic_cast<Parser::Vehicle *>(&to)) ||
           convert_unit_maybe(dynamic_cast<const Parser::Vehicle *>(&from), dynamic_cast<Parser::Biped *>(&to)) ||
           ATTEMPT_CONVERT_TYPE(Parser::Biped) ||
           ATTEMPT_CONVERT_TYPE(Parser::Vehicle) ||
           ATTEMPT_CONVERT_TYPE(Parser::Weapon) ||
           ATTEMPT_CONVERT_TYPE(Parser::Equipment) ||
           ATTEMPT_CONVERT_TYPE(Parser::Garbage) ||
           ATTEMPT_CONVERT_TYPE(Parser::Projectile) ||
           ATTEMPT_CONVERT_TYPE(Parser::Scenery) ||
           ATTEMPT_CONVERT_TYPE(Parser::DeviceMachine) ||
           ATTEMPT_CONVERT_TYPE(Parser::DeviceControl) ||
           ATTEMPT_CONVERT_TYPE(Parser::DeviceLightFixture) ||
           ATTEMPT_CONVERT_TYPE(Parser::Placeholder) ||
           ATTEMPT_CONVERT_TYPE(Parser::SoundScenery)) {
            return;
        }
        
        eprintf_error("convert_object(): Conversion error between %s and %s! This is a bug!", from.struct_name(), to.struct_name());
        std::terminate();
    }
}
