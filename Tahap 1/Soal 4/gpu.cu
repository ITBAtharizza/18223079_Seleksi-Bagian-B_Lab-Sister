#include <cuda_runtime.h>
#include <cstdint>
#include <device_launch_parameters.h>

// Pewarnaan
__device__ uchar4 mapIterationToColor(int n, int max_iter) {
    if (n == max_iter) return make_uchar4(0, 0, 0, 255);
    double t = static_cast<double>(n) / static_cast<double>(max_iter);
    uint8_t r = static_cast<uint8_t>(9 * (1 - t) * t * t * t * 255);
    uint8_t g = static_cast<uint8_t>(15 * (1 - t) * (1 - t) * t * t * 255);
    uint8_t b = static_cast<uint8_t>(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
    return make_uchar4(r, g, b, 255);
}

// Kernel Mandelbrot dengan parameter view
__global__ void mandelbrotKernel(uchar4* pixels, int width, int height, int max_iter, double viewX, double viewY, double zoom) {
    int px = blockIdx.x * blockDim.x + threadIdx.x;
    int py = blockIdx.y * blockDim.y + threadIdx.y;

    if (px >= width || py >= height) return;

    double x0 = (static_cast<double>(px) - width / 2.0) * zoom / width + viewX;
    double y0 = (static_cast<double>(py) - height / 2.0) * zoom / width + viewY;
    
    double x = 0.0, y = 0.0;
    int iteration = 0;
    while (x * x + y * y <= 4.0 && iteration < max_iter) {
        double xtemp = x * x - y * y + x0;
        y = 2.0 * x * y + y0;
        x = xtemp;
        iteration++;
    }
    
    pixels[py * width + px] = mapIterationToColor(iteration, max_iter);
}

// Kernel Julia
__global__ void juliaKernel(uchar4* pixels, int width, int height, int max_iter, double viewX, double viewY, double zoom, double cX, double cY) {
    int px = blockIdx.x * blockDim.x + threadIdx.x;
    int py = blockIdx.y * blockDim.y + threadIdx.y;

    if (px >= width || py >= height) return;

    double zx = (static_cast<double>(px) - width / 2.0) * zoom / width + viewX;
    double zy = (static_cast<double>(py) - height / 2.0) * zoom / width + viewY;
    
    int iteration = 0;
    while (zx * zx + zy * zy <= 4.0 && iteration < max_iter) {
        double xtemp = zx * zx - zy * zy + cX;
        zy = 2.0 * zx * zy + cY;
        zx = xtemp;
        iteration++;
    }

    pixels[py * width + px] = mapIterationToColor(iteration, max_iter);
}

// Kernel Mandelbrot
extern "C" void launchMandelbrotKernel(uchar4* pixels, int width, int height, int max_iter, double viewX, double viewY, double zoom) {
    dim3 threadsPerBlock(16, 16);
    dim3 numBlocks( (width + threadsPerBlock.x - 1) / threadsPerBlock.x,
                    (height + threadsPerBlock.y - 1) / threadsPerBlock.y );
    mandelbrotKernel<<<numBlocks, threadsPerBlock>>>(pixels, width, height, max_iter, viewX, viewY, zoom);
}

// Wrapper untuk kernel Julia
extern "C" void launchJuliaKernel(uchar4* pixels, int width, int height, int max_iter, double viewX, double viewY, double zoom, double cX, double cY) {
    dim3 threadsPerBlock(16, 16);
    dim3 numBlocks( (width + threadsPerBlock.x - 1) / threadsPerBlock.x,
                    (height + threadsPerBlock.y - 1) / threadsPerBlock.y );
    juliaKernel<<<numBlocks, threadsPerBlock>>>(pixels, width, height, max_iter, viewX, viewY, zoom, cX, cY);
}