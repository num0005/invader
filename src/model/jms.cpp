// SPDX-License-Identifier: GPL-3.0-only

#include <stdexcept>
#include <invader/model/jms.hpp>

namespace Invader {
#define SET_CURSOR const char *cursor = next_character(string);
#define SET_END    if(end != nullptr) { \
                       *end = cursor; \
                   }
                
    static const char CRLF[] = "\r\n";
    static const char TAB[] = "\t";
    static const constexpr std::uint16_t JMS_VERSION = 8200;
    
    // Skip to the next character that can be read
    static const char *next_character(const char *string) {
        if(string) {
            while(*string == '\r' || *string == '\t' || *string == '\n') {
                string++;
            }
            if(*string == 0) {
                throw std::invalid_argument("no character afterwards");
            }
            return string;
        }
        
        throw std::invalid_argument("null string given");
    }
    
    static std::int32_t read_next_int32(const char *&string) {
        std::size_t offset;
        std::int32_t value = std::stol(string, &offset, 10);
        string += offset;
        return value;
    }
    
    static std::int16_t read_next_int16(const char *&string) {
        auto value = read_next_int32(string);
        if(value > INT16_MAX || value < INT16_MIN) {
            throw std::out_of_range("int16 out of range");
        }
        return value;
    }
    
    static float read_next_float(const char *&string) {
        std::size_t offset;
        float value = std::stof(string, &offset);
        string += offset;
        return value;
    }
    
    static std::string string_from_string(const char *&string, bool limit_31_characters) {
        string = next_character(string);
        const char *end_of_string = string;
        while(*end_of_string && *end_of_string != '\r' && *end_of_string != '\n' && *end_of_string != '\t') {
            end_of_string++;
        }
        
        if(limit_31_characters && end_of_string - string > 31) {
            throw std::out_of_range("maximum string length exceeded");
        }
        
        std::string value = std::string(string, end_of_string);
        string = end_of_string;
        return value;
    }
    
    template <typename T> static std::string array_to_string(const std::vector<T> &vector) {
        std::string rval = std::to_string(vector.size()) + CRLF;
        for(auto &i : vector) {
            rval += i.string() + CRLF;
        }
        return rval;
    }
    
    template <typename T> static std::vector<T> array_from_string(const char *&string) {
        std::vector<T> arr;
        auto count = read_next_int16(string);
        if(count < 0) {
            throw std::out_of_range("negative array size");
        }
        std::size_t countv = static_cast<std::size_t>(count);
        arr.reserve(countv);
        for(std::size_t i = 0; i < countv; i++) {
            arr.emplace_back(T::from_string(string, &string));
        }
        return arr;
    }
    
    static std::string vector_to_string(const HEK::Vector3D<HEK::NativeEndian> &vector) {
        return std::to_string(vector.i.read()) + TAB + std::to_string(vector.j.read()) + TAB + std::to_string(vector.k.read());
    }
    
    static std::string vector_to_string(const HEK::Point3D<HEK::NativeEndian> &vector) {
        return std::to_string(vector.x.read()) + TAB + std::to_string(vector.y.read()) + TAB + std::to_string(vector.z.read());
    }
    
    static std::string vector_to_string(const HEK::Point2D<HEK::NativeEndian> &vector) {
        return std::to_string(vector.x.read()) + TAB + std::to_string(vector.y.read());
    }
    
    static std::string vector_to_string(const HEK::Quaternion<HEK::NativeEndian> &vector) {
        return std::to_string(vector.i.read()) + TAB + std::to_string(vector.j.read())+ TAB + std::to_string(vector.k.read())+ TAB + std::to_string(vector.w.read());
    }
    
    static HEK::Quaternion<HEK::NativeEndian> quaternion_from_string(const char *&string) {
        HEK::Quaternion<HEK::NativeEndian> v;
        v.i = read_next_float(string);
        v.j = read_next_float(string);
        v.k = read_next_float(string);
        v.w = read_next_float(string);
        return v;
    }
    
    static HEK::Vector3D<HEK::NativeEndian> vector3d_from_string(const char *&string) {
        HEK::Vector3D<HEK::NativeEndian> v;
        v.i = read_next_float(string);
        v.j = read_next_float(string);
        v.k = read_next_float(string);
        return v;
    }
    
    static HEK::Point2D<HEK::NativeEndian> point2d_from_string(const char *&string) {
        HEK::Point2D<HEK::NativeEndian> v;
        v.x = read_next_float(string);
        v.y = read_next_float(string);
        return v;
    }
    
    static HEK::Point3D<HEK::NativeEndian> point3d_from_string(const char *&string) {
        HEK::Point3D<HEK::NativeEndian> v;
        v.x = read_next_float(string);
        v.y = read_next_float(string);
        v.z = read_next_float(string);
        return v;
    }
    
    JMS JMS::from_string(const char *string, const char **end) {
        try {
            SET_CURSOR
            auto version = read_next_int32(cursor);
            if(version != JMS_VERSION) {
                throw std::invalid_argument("invalid version");
            }
            read_next_int32(cursor); // skip
            
            // Build our JMS struct
            JMS jms;
            jms.nodes = array_from_string<Node>(cursor);
            jms.materials = array_from_string<Material>(cursor);
            jms.markers = array_from_string<Marker>(cursor);
            jms.regions = array_from_string<Region>(cursor);
            jms.vertices = array_from_string<Vertex>(cursor);
            jms.triangles = array_from_string<Triangle>(cursor);
            SET_END
            return jms;
        }
        catch (std::exception &) {
            throw;
        }
    }
    std::string JMS::string() const {
        std::string r;
        
        r += std::to_string(JMS_VERSION) + CRLF;
        r += std::to_string(0) + CRLF;
        r += array_to_string(nodes);
        r += array_to_string(materials);
        r += array_to_string(markers);
        r += array_to_string(regions);
        r += array_to_string(vertices);
        r += array_to_string(triangles);
        
        return r;
    }
    
    JMS::Marker JMS::Marker::from_string(const char *string, const char **end) {
        SET_CURSOR
        Marker m;
        m.name = string_from_string(cursor, true);
        m.region = read_next_int16(cursor);
        m.node = read_next_int16(cursor);
        m.rotation = quaternion_from_string(cursor);
        m.position = point3d_from_string(cursor) / 100.0F;
        m.radius = read_next_float(cursor);
        SET_END
        return m;
    }
    std::string JMS::Marker::string() const {
        return this->name + CRLF +
               std::to_string(static_cast<std::int16_t>(this->region)) + CRLF +
               std::to_string(static_cast<std::int16_t>(this->node)) + CRLF +
               vector_to_string(this->rotation) + CRLF +
               vector_to_string(this->position * 100.0F) + CRLF +
               std::to_string(this->radius);
    }
    
    JMS::Node JMS::Node::from_string(const char *string, const char **end) {
        SET_CURSOR
        Node n;
        n.name = string_from_string(cursor, true);
        n.first_child = read_next_int16(cursor);
        n.sibling_node = read_next_int16(cursor);
        n.rotation = quaternion_from_string(cursor);
        n.position = point3d_from_string(cursor) / 100.0F;
        SET_END
        return n;
    }
    std::string JMS::Node::string() const {
        return this->name + CRLF +
               std::to_string(static_cast<std::int16_t>(this->first_child)) + CRLF +
               std::to_string(static_cast<std::int16_t>(this->sibling_node)) + CRLF +
               vector_to_string(this->rotation) + CRLF +
               vector_to_string(this->position * 100.0F);
    }
    
    JMS::Material JMS::Material::from_string(const char *string, const char **end) {
        SET_CURSOR
        Material m;
        m.name = string_from_string(cursor, false);
        m.tif_path = string_from_string(cursor, false);
        SET_END
        return m;
    }
    std::string JMS::Material::string() const {
        return this->name + CRLF + this->tif_path;
    }
    
    JMS::Region JMS::Region::from_string(const char *string, const char **end) {
        SET_CURSOR
        Region r;
        r.name = string_from_string(cursor, true);
        SET_END
        return r;
    }
    std::string JMS::Region::string() const {
        return this->name + CRLF;
    }
    
    JMS::Vertex JMS::Vertex::from_string(const char *string, const char **end) {
        SET_CURSOR
        Vertex v;
        v.node0 = read_next_int16(cursor);
        v.position = point3d_from_string(cursor) / 100.0F;
        v.normal = vector3d_from_string(cursor).normalize();
        v.node1 = read_next_int16(cursor);
        v.node1_weight = read_next_float(cursor);
        v.texture_coordinates = point2d_from_string(cursor);
        v.texture_coordinates.y = 1.0F - v.texture_coordinates.y; // this is flipped for some reason
        read_next_float(cursor);
        SET_END
        return v;
    }
    std::string JMS::Vertex::string() const {
        auto modified_texture_coordinates = this->texture_coordinates;
        modified_texture_coordinates.y = 1.0F - modified_texture_coordinates.y;
        
        return std::to_string(static_cast<std::int16_t>(this->node0)) + CRLF +
               vector_to_string(this->position * 100.0F) + CRLF +
               vector_to_string(this->normal) + CRLF +
               std::to_string(static_cast<std::int16_t>(this->node1)) + CRLF +
               std::to_string(this->node1_weight) + CRLF +
               vector_to_string(modified_texture_coordinates) + TAB + "0";
    }
    
    JMS::Triangle JMS::Triangle::from_string(const char *string, const char **end) {
        SET_CURSOR
        Triangle t;
        t.region = read_next_int16(cursor);
        t.shader = read_next_int16(cursor);
        t.vertices[0] = read_next_int16(cursor);
        t.vertices[2] = read_next_int16(cursor);
        t.vertices[1] = read_next_int16(cursor);
        SET_END
        return t;
    }
    std::string JMS::Triangle::string() const {
        return std::to_string(static_cast<std::int16_t>(this->region)) + CRLF +
               std::to_string(static_cast<std::int16_t>(this->shader)) + CRLF +
               std::to_string(static_cast<std::int16_t>(this->vertices[0])) + TAB +
               std::to_string(static_cast<std::int16_t>(this->vertices[2])) + TAB +
               std::to_string(static_cast<std::int16_t>(this->vertices[1]));
    }
}