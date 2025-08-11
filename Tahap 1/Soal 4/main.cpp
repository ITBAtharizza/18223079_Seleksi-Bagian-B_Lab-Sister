#include <iostream>
#include <vector>
#include <cstdint>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <optional>
#include <omp.h>
#include <cuda_runtime.h>
#include <SFML/Graphics.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Fungsi-fungsi
extern "C" void launchMandelbrotKernel(uchar4* pixels, int width, int height, int max_iter, double viewX, double viewY, double zoom);
extern "C" void launchJuliaKernel(uchar4* pixels, int width, int height, int max_iter, double viewX, double viewY, double zoom, double cX, double cY);

// Error checking CUDA
#define CUDA_CHECK(err) { \
    cudaError_t err_ = (err); \
    if (err_ != cudaSuccess) { \
        std::cerr << "CUDA Error: " << cudaGetErrorString(err_) << " in " << __FILE__ << " at line " << __LINE__ << std::endl; \
        exit(EXIT_FAILURE); \
    } \
}

// Logika CPU
struct Color { uint8_t r, g, b; };
Color mapIterationToColorCPU(int n, int max_iter) {
    if (n == max_iter) return {0, 0, 0};
    double t = static_cast<double>(n) / static_cast<double>(max_iter);
    uint8_t r = static_cast<uint8_t>(9 * (1 - t) * t * t * t * 255);
    uint8_t g = static_cast<uint8_t>(15 * (1 - t) * (1 - t) * t * t * 255);
    uint8_t b = static_cast<uint8_t>(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
    return {r, g, b};
}

// Benchmarking
void run_benchmarks(int width, int height, int max_iterations) {
    std::cout << "==================================================" << std::endl;
    std::cout << "                 MEMULAI BENCHMARK                " << std::endl;
    std::cout << "==================================================\n" << std::endl;

    double serial_time = 0.0, parallel_cpu_time = 0.0, gpu_time = 0.0;

    // SERIAL (CPU)
    {
        std::cout << "Memulai generasi SERIAL (CPU)..." << std::endl;
        std::vector<uint8_t> pixels(static_cast<size_t>(width) * height * 3);
        auto start = std::chrono::high_resolution_clock::now();
        for (int py = 0; py < height; ++py) {
            for (int px = 0; px < width; ++px) {
                double x0 = (static_cast<double>(px) - width / 2.0) * 3.5 / width - 0.75;
                double y0 = (static_cast<double>(py) - height / 2.0) * 3.5 / width;
                double x = 0.0, y = 0.0; int iteration = 0;
                while (x * x + y * y <= 4.0 && iteration < max_iterations) {
                    double xtemp = x * x - y * y + x0; y = 2.0 * x * y + y0; x = xtemp; iteration++;
                }
                Color color = mapIterationToColorCPU(iteration, max_iterations);
                int index = (py * width + px) * 3;
                pixels[index] = color.r; pixels[index + 1] = color.g; pixels[index + 2] = color.b;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        serial_time = std::chrono::duration<double>(end - start).count();
        std::cout << "Waktu eksekusi SERIAL: " << serial_time << " detik" << std::endl;
        stbi_write_png("output/benchmark_serial.png", width, height, 3, pixels.data(), width * 3);
    }

    // PARALEL (CPU OpenMP)
    {
        std::cout << "\nMemulai generasi PARALEL (CPU OpenMP)..." << std::endl;
        std::vector<uint8_t> pixels(static_cast<size_t>(width) * height * 3);
        auto start = std::chrono::high_resolution_clock::now();
        #pragma omp parallel for schedule(dynamic)
        for (int py = 0; py < height; ++py) {
            for (int px = 0; px < width; ++px) {
                double x0 = (static_cast<double>(px) - width / 2.0) * 3.5 / width - 0.75;
                double y0 = (static_cast<double>(py) - height / 2.0) * 3.5 / width;
                double x = 0.0, y = 0.0; int iteration = 0;
                while (x * x + y * y <= 4.0 && iteration < max_iterations) {
                    double xtemp = x * x - y * y + x0; y = 2.0 * x * y + y0; x = xtemp; iteration++;
                }
                Color color = mapIterationToColorCPU(iteration, max_iterations);
                int index = (py * width + px) * 3;
                pixels[index] = color.r; pixels[index + 1] = color.g; pixels[index + 2] = color.b;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        parallel_cpu_time = std::chrono::duration<double>(end - start).count();
        std::cout << "Waktu eksekusi PARALEL CPU: " << parallel_cpu_time << " detik" << std::endl;
        stbi_write_png("output/benchmark_parallel_cpu.png", width, height, 3, pixels.data(), width * 3);
    }

    // PARALEL (GPU CUDA)
    {
        std::cout << "\nMemulai generasi PARALEL (GPU CUDA)..." << std::endl;
        const size_t num_pixels = static_cast<size_t>(width) * height;
        const size_t bytes = num_pixels * sizeof(uchar4);
        std::vector<uchar4> host_pixels(num_pixels);
        uchar4* device_pixels = nullptr;
        CUDA_CHECK(cudaMalloc(&device_pixels, bytes));
        auto start = std::chrono::high_resolution_clock::now();
        launchMandelbrotKernel(device_pixels, width, height, max_iterations, -0.7, 0.0, 3.5);
        CUDA_CHECK(cudaDeviceSynchronize());
        CUDA_CHECK(cudaMemcpy(host_pixels.data(), device_pixels, bytes, cudaMemcpyDeviceToHost));
        auto end = std::chrono::high_resolution_clock::now();
        gpu_time = std::chrono::duration<double>(end - start).count();
        std::cout << "Waktu eksekusi PARALEL GPU: " << gpu_time << " detik" << std::endl;
        stbi_write_png("output/benchmark_parallel_gpu.png", width, height, 4, host_pixels.data(), width * 4);
        CUDA_CHECK(cudaFree(device_pixels));
    }

    // HASIL 
    std::cout << "\n--- Hasil Benchmark ---" << std::endl;
    std::cout << "Waktu Serial CPU: \t\t" << serial_time << " detik" << std::endl;
    std::cout << "Waktu Paralel CPU (OpenMP):\t" << parallel_cpu_time << " detik" << std::endl;
    std::cout << "Waktu Paralel GPU (CUDA):\t" << gpu_time << " detik" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "Speedup (CPU Paralel vs CPU Serial):\t" << serial_time / parallel_cpu_time << "x" << std::endl;
    std::cout << "Speedup (GPU vs CPU Serial):\t\t" << serial_time / gpu_time << "x" << std::endl;
    std::cout << "Speedup (GPU vs CPU Paralel):\t\t" << parallel_cpu_time / gpu_time << "x" << std::endl;
}

// Interactive GUI
void run_gui(int computeWidth, int computeHeight) {
    std::cout << "\n==================================================" << std::endl;
    std::cout << "                   GUI Interaktif                   " << std::endl;
    std::cout << "==================================================\n" << std::endl;
    const unsigned int WINDOW_WIDTH = 1280;
    const unsigned int WINDOW_HEIGHT = 720;
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Mandelbrot of Madness");
    window.setFramerateLimit(60);
    double viewX = -0.7, viewY = 0.0, zoom = 2.5;
    int max_iterations_gui = 250;
    bool needsRedraw = true, isPanning = false;
    sf::Vector2i panStartPos;
    enum FractalMode { MANDELBROT, JULIA } currentMode = MANDELBROT;
    double juliaCX = -0.8, juliaCY = 0.156;
    std::vector<uchar4> host_pixels(static_cast<size_t>(computeWidth) * computeHeight);
    uchar4* device_pixels = nullptr;
    CUDA_CHECK(cudaMalloc(&device_pixels, host_pixels.size() * sizeof(uchar4)));
    sf::Image image;
    sf::Texture texture;
    sf::Sprite sprite;
    texture.create(computeWidth, computeHeight);
    sprite.setTexture(texture);
    float scaleX = static_cast<float>(WINDOW_WIDTH) / computeWidth;
    float scaleY = static_cast<float>(WINDOW_HEIGHT) / computeHeight;
    sprite.setScale(scaleX, scaleY);
    auto mapPixelToComplex = [&](int mouseX, int mouseY) {
        double scaledMouseX = mouseX / scaleX;
        double scaledMouseY = mouseY / scaleY;
        double real = (scaledMouseX - computeWidth / 2.0) * zoom / computeWidth + viewX;
        double imag = (scaledMouseY - computeHeight / 2.0) * zoom / computeWidth + viewY;
        return sf::Vector2<double>(real, imag);
    };
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0) zoom *= 0.9; else zoom *= 1.1; needsRedraw = true;
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                isPanning = true; panStartPos = sf::Mouse::getPosition(window);
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                isPanning = false;
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F) {
                currentMode = (currentMode == MANDELBROT) ? JULIA : MANDELBROT; needsRedraw = true;
            }
            if (event.type == sf::Event::MouseMoved) {
                if (isPanning) {
                    sf::Vector2i currentPos = sf::Mouse::getPosition(window);
                    sf::Vector2i delta = panStartPos - currentPos;
                    viewX += static_cast<double>(delta.x) * zoom / WINDOW_WIDTH;
                    viewY += static_cast<double>(delta.y) * zoom / WINDOW_WIDTH;
                    panStartPos = currentPos; needsRedraw = true;
                }
                if (currentMode == JULIA) {
                    sf::Vector2<double> c = mapPixelToComplex(event.mouseMove.x, event.mouseMove.y);
                    juliaCX = c.x; juliaCY = c.y; needsRedraw = true;
                }
            }
        }
        if (needsRedraw) {
            if (currentMode == MANDELBROT) {
                launchMandelbrotKernel(device_pixels, computeWidth, computeHeight, max_iterations_gui, viewX, viewY, zoom);
            } else {
                launchJuliaKernel(device_pixels, computeWidth, computeHeight, max_iterations_gui, viewX, viewY, zoom, juliaCX, juliaCY);
            }
            CUDA_CHECK(cudaDeviceSynchronize());
            CUDA_CHECK(cudaMemcpy(host_pixels.data(), device_pixels, host_pixels.size() * sizeof(uchar4), cudaMemcpyDeviceToHost));
            image.create(computeWidth, computeHeight, reinterpret_cast<const sf::Uint8*>(host_pixels.data()));
            texture.loadFromImage(image);
            needsRedraw = false;
        }
        window.clear();
        window.draw(sprite);
        window.display();
    }
    CUDA_CHECK(cudaFree(device_pixels));
}

// Main
int main(int argc, char* argv[]) {
    int computeWidth = 1280, computeHeight = 720, max_iterations = 1000;

    if (argc >= 3) {
        computeWidth = std::atoi(argv[1]);
        computeHeight = std::atoi(argv[2]);
    } else {
        std::cout << "Cara pakai: " << argv[0] << " [lebar] [tinggi] (opsional)" << std::endl;
    }
    std::cout << "Menggunakan resolusi komputasi: " << computeWidth << "x" << computeHeight << "\n" << std::endl;

    run_benchmarks(computeWidth, computeHeight, max_iterations);

    std::cout << "\n--- Kontrol GUI Interaktif ---" << std::endl;
    std::cout << "- Scroll Mouse\t\t: Zoom In / Out" << std::endl;
    std::cout << "- Klik Kiri + Geser\t: Menggeser Tampilan (Pan)" << std::endl;
    std::cout << "- Tekan 'F'\t\t: Beralih mode Mandelbrot / Julia" << std::endl;
    std::cout << "- Gerakkan Mouse\t: (Mode Julia) Mengubah bentuk fraktal secara real-time\n" << std::endl;

    std::cout << "Benchmark Selesai. Tekan Enter untuk meluncurkan GUI interaktif...";
    std::cin.get();

    run_gui(computeWidth, computeHeight);

    return 0;
}