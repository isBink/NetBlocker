#pragma once
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <cctype>
#include <iostream>

class json {
public:
    enum class value_type { null, object, array, string, number, boolean };

    json() : type(value_type::null) {}
    json(const char* str) : type(value_type::string), str_value(str) {}
    json(const std::string& str) : type(value_type::string), str_value(str) {}
    json(int val) : type(value_type::number), double_value(val) {}
    json(bool val) : type(value_type::boolean), bool_value(val) {}
    json(double val) : type(value_type::number), double_value(val) {}

    // Assignment operators for numbers
    json& operator=(int val) {
        type = value_type::number;
        double_value = val;
        return *this;
    }
    json& operator=(double val) {
        type = value_type::number;
        double_value = val;
        return *this;
    }
    json& operator=(float val) {
        return operator=(static_cast<double>(val));
    }

    json& operator[](const std::string& key) {
        if (type != value_type::object) {
            type = value_type::object;
            object_values.clear();
        }
        return object_values[key];
    }

    const json& operator[](const std::string& key) const {
        static json null_json;
        if (type == value_type::object) {
            auto it = object_values.find(key);
            if (it != object_values.end()) return it->second;
        }
        return null_json;
    }

    json& operator[](size_t index) {
        if (type != value_type::array) {
            type = value_type::array;
            array_values.clear();
        }
        if (index >= array_values.size()) {
            array_values.resize(index + 1);
        }
        return array_values[index];
    }

    const json& operator[](size_t index) const {
        static json null_json;
        if (type == value_type::array && index < array_values.size()) {
            return array_values[index];
        }
        return null_json;
    }

    bool is_array() const { return type == value_type::array; }
    bool is_object() const { return type == value_type::object; }
    size_t size() const { return is_array() ? array_values.size() : 0; }

    template<typename T>
    T get() const;

    std::string dump() const {
        switch (type) {
        case value_type::null: return "null";
        case value_type::string: return "\"" + escape_string(str_value) + "\"";
        case value_type::number: return std::to_string(double_value);
        case value_type::boolean: return bool_value ? "true" : "false";
        case value_type::object: {
            std::string result = "{";
            bool first = true;
            for (const auto& kv : object_values) {
                if (!first) result += ",";
                result += "\"" + escape_string(kv.first) + "\":" + kv.second.dump();
                first = false;
            }
            result += "}";
            return result;
        }
        case value_type::array: {
            std::string result = "[";
            bool first = true;
            for (const auto& el : array_values) {
                if (!first) result += ",";
                result += el.dump();
                first = false;
            }
            result += "]";
            return result;
        }
        }
        return "";
    }

    // Pretty print with indent
    std::string dump(int indent) const {
        return dump_formatted(indent, 0);
    }

    static json parse(const std::string& s) {
        size_t pos = 0;
        return parse_value(s, pos);
    }

    using iterator = std::vector<json>::iterator;
    using const_iterator = std::vector<json>::const_iterator;

    iterator begin() { return is_array() ? array_values.begin() : empty_array.begin(); }
    iterator end() { return is_array() ? array_values.end() : empty_array.end(); }
    const_iterator begin() const { return is_array() ? array_values.cbegin() : empty_array.cbegin(); }
    const_iterator end() const { return is_array() ? array_values.cend() : empty_array.cend(); }

    bool contains(const std::string& key) const {
        return type == value_type::object && object_values.find(key) != object_values.end();
    }

private:
    value_type type;
    std::string str_value;
    double double_value = 0.0;
    bool bool_value = false;
    std::map<std::string, json> object_values;
    std::vector<json> array_values;
    static inline std::vector<json> empty_array;

    // Skip whitespace characters during parsing
    static void skip_whitespace(const std::string& s, size_t& pos) {
        while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos]))) pos++;
    }

    // Parse a JSON string value, handling escape sequences
    static std::string parse_string(const std::string& s, size_t& pos) {
        std::string result;
        if (pos >= s.size() || s[pos] != '"') return result;
        pos++;
        while (pos < s.size()) {
            char c = s[pos];
            if (c == '"') { pos++; break; }
            if (c == '\\') {
                pos++;
                if (pos >= s.size()) break;
                char esc = s[pos++];
                switch (esc) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                default: result += esc; break;
                }
            }
            else {
                result += c;
                pos++;
            }
        }
        return result;
    }

    // Parse a JSON number (integer or floating point)
    static double parse_number(const std::string& s, size_t& pos) {
        size_t start = pos;
        if (s[pos] == '-') pos++;
        while (pos < s.size() && (std::isdigit(static_cast<unsigned char>(s[pos])) || s[pos] == '.')) pos++;
        return std::stod(s.substr(start, pos - start));
    }

    static json parse_value(const std::string& s, size_t& pos) {
        skip_whitespace(s, pos);
        if (pos >= s.size()) return json();

        switch (s[pos]) {
        case '{': return parse_object(s, pos);
        case '[': return parse_array(s, pos);
        case '"': return json(parse_string(s, pos));
        default:
            if (s.compare(pos, 4, "true") == 0) { pos += 4; return json(true); }
            if (s.compare(pos, 5, "false") == 0) { pos += 5; return json(false); }
            if (s.compare(pos, 4, "null") == 0) { pos += 4; return json(); }
            if (s[pos] == '-' || std::isdigit(static_cast<unsigned char>(s[pos]))) return json(parse_number(s, pos));
        }
        return json();
    }

    static json parse_object(const std::string& s, size_t& pos) {
        json result;
        result.type = value_type::object;
        pos++;
        while (true) {
            skip_whitespace(s, pos);
            if (pos >= s.size() || s[pos] == '}') { pos++; break; }
            std::string key = parse_string(s, pos);
            skip_whitespace(s, pos);
            if (pos >= s.size() || s[pos] != ':') break;
            pos++;
            skip_whitespace(s, pos);
            result.object_values[key] = parse_value(s, pos);
            skip_whitespace(s, pos);
            if (pos < s.size() && s[pos] == ',') pos++;
        }
        return result;
    }

    static json parse_array(const std::string& s, size_t& pos) {
        json result;
        result.type = value_type::array;
        pos++;
        while (true) {
            skip_whitespace(s, pos);
            if (pos >= s.size() || s[pos] == ']') { pos++; break; }
            result.array_values.push_back(parse_value(s, pos));
            skip_whitespace(s, pos);
            if (pos < s.size() && s[pos] == ',') pos++;
        }
        return result;
    }

    std::string dump_formatted(int indent, int depth) const {
        std::string spacing(depth * indent, ' ');
        std::string nextSpacing((depth + 1) * indent, ' ');

        switch (type) {
        case value_type::null: return "null";
        case value_type::string: return "\"" + escape_string(str_value) + "\"";
        case value_type::number: return std::to_string(double_value);
        case value_type::boolean: return bool_value ? "true" : "false";
        case value_type::object: {
            std::string result = "{\n";
            bool first = true;
            for (const auto& kv : object_values) {
                if (!first) result += ",\n";
                result += nextSpacing + "\"" + escape_string(kv.first) + "\": " + kv.second.dump_formatted(indent, depth + 1);
                first = false;
            }
            result += "\n" + spacing + "}";
            return result;
        }
        case value_type::array: {
            std::string result = "[\n";
            bool first = true;
            for (const auto& el : array_values) {
                if (!first) result += ",\n";
                result += nextSpacing + el.dump_formatted(indent, depth + 1);
                first = false;
            }
            result += "\n" + spacing + "]";
            return result;
        }
        }
        return "";
    }

    static std::string escape_string(const std::string& s) {
        std::string out;
        for (char c : s) {
            switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
            }
        }
        return out;
    }
};

// Template specializations

template<>
inline std::string json::get<std::string>() const {
    return type == value_type::string ? str_value : std::string{};
}

template<>
inline int json::get<int>() const {
    return type == value_type::number ? static_cast<int>(double_value) : 0;
}

template<>
inline double json::get<double>() const {
    return type == value_type::number ? double_value : 0.0;
}

template<>
inline bool json::get<bool>() const {
    return type == value_type::boolean ? bool_value : false;
}

template<>
inline json json::get<json>() const {
    return *this;
}

inline std::istream& operator>>(std::istream& is, json& j) {
    std::ostringstream ss;
    ss << is.rdbuf();  // Read entire stream into string
    std::string content = ss.str();
    j = json::parse(content);
    return is;
}
