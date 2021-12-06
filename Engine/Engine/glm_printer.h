#pragma once

#include <glm/glm.hpp>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace glm {

inline void to_json(nlohmann::json& j, const mat4& matrix) {
    j = { { "matrix[0][0]", matrix[0][0] }, { "matrix[1][0]", matrix[1][0] }, { "matrix[2][0]", matrix[2][0] }, { "matrix[3][0]", matrix[3][0] },
          { "matrix[0][1]", matrix[0][1] }, { "matrix[1][1]", matrix[1][1] }, { "matrix[2][1]", matrix[2][1] }, { "matrix[3][1]", matrix[3][1] },
          { "matrix[0][2]", matrix[0][2] }, { "matrix[1][2]", matrix[1][2] }, { "matrix[2][2]", matrix[2][2] }, { "matrix[3][2]", matrix[3][2] },
          { "matrix[0][3]", matrix[0][3] }, { "matrix[1][3]", matrix[1][3] }, { "matrix[2][3]", matrix[2][3] }, { "matrix[3][3]", matrix[3][3] }
    };
}

inline void from_json(const nlohmann::json& j, mat4& matrix) {
    j.at("matrix[0][0]").get_to(matrix[0][0]);
    j.at("matrix[1][0]").get_to(matrix[1][0]);
    j.at("matrix[2][0]").get_to(matrix[2][0]);
    j.at("matrix[3][0]").get_to(matrix[3][0]);

    j.at("matrix[0][1]").get_to(matrix[0][1]);
    j.at("matrix[1][1]").get_to(matrix[1][1]);
    j.at("matrix[2][1]").get_to(matrix[2][1]);
    j.at("matrix[3][1]").get_to(matrix[3][1]);

    j.at("matrix[0][2]").get_to(matrix[0][2]);
    j.at("matrix[1][2]").get_to(matrix[1][2]);
    j.at("matrix[2][2]").get_to(matrix[2][2]);
    j.at("matrix[3][2]").get_to(matrix[3][2]);

    j.at("matrix[0][3]").get_to(matrix[0][3]);
    j.at("matrix[1][3]").get_to(matrix[1][3]);
    j.at("matrix[2][3]").get_to(matrix[2][3]);
    j.at("matrix[3][3]").get_to(matrix[3][3]);
}

inline void to_json(nlohmann::json& j, const vec3& vector) {
    j = { { "vector[0]", vector[0] }, { "vector[1]", vector[1] }, { "vector[2]", vector[2] }
    };
}

inline void from_json(const nlohmann::json& j, vec3& vector) {
    j.at("vector[0]").get_to(vector[0]);
    j.at("vector[1]").get_to(vector[1]);
    j.at("vector[2]").get_to(vector[2]);
}

} // namespace glm {

namespace std::filesystem {
    inline void to_json(nlohmann::json& j, const path& opt) {
        j = opt.string();
    }

    inline void from_json(const nlohmann::json& j, path& opt) {
        opt = j.get<std::string>();
    }
} // namespace filesystem
