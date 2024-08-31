// #include <iostream>
// #include <vector>
// #include <Eigen/Dense>
// #include "PLYReader.h"
// #include "PLYWriter.h"
// #include "BilateralFilter.h"

// int main() {
//     const std::string input_filename = "OriginalMesh.ply";
//     const std::string output_filename = "FilteredMesh.ply";
//     const int width = 2592;
//     const int height = 1944;

//     std::vector<Eigen::Vector3f> vertices;
//     std::vector<Eigen::Vector3f> normals;

//     PLYReader reader(input_filename);
//     reader.readPLY(vertices, normals, width, height);

//     BilateralFilter filter(1.0f, 1.0f);
//     filter.applyFilter(vertices, normals, width, height);

//     PLYWriter writer(output_filename);
//     writer.writePLY(vertices, normals, width, height, true);

//     std::cout << "Filtering completed and result written to " << output_filename << std::endl;
//     return 0;
// }

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>

struct Point {
    float x, y, z, nx, ny, nz;
};

std::vector<Point> readPLYFile(const std::string& filename, int& width, int& height) {
    std::vector<Point> points;
    std::ifstream file(filename);
    std::string line;
    bool readingPoints = false;
    width = height = 0;

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return points;
    }

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        if (line.find("element vertex") != std::string::npos) {
            size_t numVertices;
            iss >> std::ws >> numVertices;
            points.resize(numVertices);
        } else if (line.find("element face") != std::string::npos) {
            // Skip face data
            continue;
        } else if (line.find("end_header") != std::string::npos) {
            readingPoints = true;
        } else if (readingPoints) {
            Point p;
            if (iss >> p.x >> p.y >> p.z >> p.nx >> p.ny >> p.nz) {
                points.push_back(p);
            }
        }
    }

    file.close();
    return points;
}

void writePLYFile(const std::string& filename, const std::vector<Point>& points) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    file << "ply\nformat ascii 1.0\n";
    file << "element vertex " << points.size() << "\n";
    file << "property float x\nproperty float y\nproperty float z\n";
    file << "property float nx\nproperty float ny\nproperty float nz\n";
    file << "element face 0\n";
    file << "property list uchar int vertex_index\n";
    file << "end_header\n";

    for (const auto& p : points) {
        file << p.x << " " << p.y << " " << p.z << " "
             << p.nx << " " << p.ny << " " << p.nz << "\n";
    }

    file.close();
}

void bilateralFilter(std::vector<Point>& points, int width, int height, float sigma_c, float sigma_s, int iterations) {
    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};

    std::vector<Point> filteredPoints = points;

    for (int iter = 0; iter < iterations; ++iter) {
        std::vector<Point> tempPoints = filteredPoints;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int index = y * width + x;

                if (std::isnan(points[index].x)) continue;

                Point& p = tempPoints[index];
                float sum_x = 0, sum_y = 0, sum_z = 0, sum_weight = 0;

                for (int k = 0; k < 4; ++k) {
                    int nx = x + dx[k];
                    int ny = y + dy[k];

                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        int neighborIndex = ny * width + nx;
                        if (std::isnan(points[neighborIndex].x)) continue;

                        Point& np = points[neighborIndex];
                        float dist = std::sqrt((p.x - np.x) * (p.x - np.x) + (p.y - np.y) * (p.y - np.y) + (p.z - np.z) * (p.z - np.z));
                        float intensity_diff = std::sqrt((p.nx - np.nx) * (p.nx - np.nx) + (p.ny - np.ny) * (p.ny - np.ny) + (p.nz - np.nz) * (p.nz - np.nz));
                        float weight = std::exp(-dist * dist / (2 * sigma_c * sigma_c)) * std::exp(-intensity_diff * intensity_diff / (2 * sigma_s * sigma_s));

                        sum_x += np.x * weight;
                        sum_y += np.y * weight;
                        sum_z += np.z * weight;
                        sum_weight += weight;
                    }
                }

                if (sum_weight > 0) {
                    p.x = sum_x / sum_weight;
                    p.y = sum_y / sum_weight;
                    p.z = sum_z / sum_weight;
                }
            }
        }

        filteredPoints = tempPoints;
    }

    points = filteredPoints;
}

int main() {
    int width, height;
    std::vector<Point> points = readPLYFile("OriginalMesh.ply", width, height);

    if (points.empty()) {
        std::cerr << "Failed to read points from file." << std::endl;
        return -1;
    }

    float sigma_c = 1.0f; // Adjust this parameter
    float sigma_s = 0.1f; // Adjust this parameter
    int iterations = 5; // Number of filter iterations

    bilateralFilter(points, width, height, sigma_c, sigma_s, iterations);
    writePLYFile("FilteredMesh.ply", points);

    return 0;
}
