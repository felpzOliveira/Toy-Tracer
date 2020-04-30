#include <cutil.h>
#include <stdio.h>
#include <iostream>
#include <ctime>

Memory global_memory = {0};

void _check(cudaError_t err, int line, const char *filename){
    if(err != cudaSuccess){
        std::cout << "CUDA error > " << filename << ": " << line << "[" << cudaGetErrorString(err) << "]" << std::endl;
        getchar();
        exit(0);
    }
}

double to_cpu_time(clock_t start, clock_t end){
    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

std::string get_time_unit(double *inval){
    std::string unit("s");
    double val = *inval;
    if(val > 60){
        unit = "min";
        val /= 60.0;
    }
    
    if(val > 60){
        unit = "h";
        val /= 60;
    }
    
    *inval = val;
    return unit;
}

int cudaInit(){
    int nDevices;    
    int dev;
    cudaGetDeviceCount(&nDevices);
    for (int i = 0; i < nDevices; i++) {
        cudaDeviceProp prop;
        cudaGetDeviceProperties(&prop, i);
        printf(" > Device name: %s\n", prop.name);
        printf(" > Memory Clock Rate (KHz): %d\n",
               prop.memoryClockRate);
        printf(" > Memory Bus Width (bits): %d\n",
               prop.memoryBusWidth);
        printf(" > Peak Memory Bandwidth (GB/s): %f\n",
               2.0*prop.memoryClockRate*(prop.memoryBusWidth/8)/1.0e6);
    }
    
    if(nDevices > 0){
        cudaDeviceProp prop;
        memset(&prop, 0, sizeof(cudaDeviceProp));
        prop.major = 1; prop.minor = 0;
        CUCHECK(cudaChooseDevice(&dev, &prop));
        CUCHECK(cudaGetDeviceProperties(&prop, dev));
        global_memory.allocated = 0;
        std::cout << "Using device " << prop.name << " [ " <<  prop.major << "." << prop.minor << " ]" << std::endl; 
        
        void *ptr = NULL;
        
        clock_t start = clock();       
        CUCHECK(cudaMalloc(&ptr, 1));
        clock_t mid = clock();
        
        cudaDeviceReset();
        clock_t end = clock();
        
        double cpu_time_mid = to_cpu_time(start, mid);
        double cpu_time_reset = to_cpu_time(mid, end);
        double cpu_time_end = to_cpu_time(start, end);
        
        std::string unitAlloc = get_time_unit(&cpu_time_mid);
        std::string unitReset = get_time_unit(&cpu_time_reset);
        std::string unitTotal = get_time_unit(&cpu_time_end);
        
        std::string state("[OK]");
        if(cpu_time_end > 1.5){
            state = "[SLOW]";
        }
        
        std::cout << "GPU init stats " << state << "\n" <<  
            " > Allocation: " << cpu_time_mid << " " << unitAlloc << std::endl;
        std::cout << " > Reset: " << cpu_time_reset << " " << unitReset << std::endl;
        std::cout << " > Global: " << cpu_time_end << " " << unitTotal << std::endl;
        
    }
    
	return dev;
}

void cudaPrintMemoryTaken(){
    std::string unity("b");
    float amount = (float)(global_memory.allocated);
    if(amount > 1024){
        amount /= 1024.f;
        unity = "KB";
    }
    
    if(amount > 1024){
        amount /= 1024.f;
        unity = "MB";
    }
    
    if(amount > 1024){
        amount /= 1024.f;
        unity = "GB";
    }
    
    std::cout << "Took " << amount << " " << unity << " of GPU memory" << std::endl;
}

int cudaSynchronize(){
    int rv = 0;
    cudaError_t errSync = cudaGetLastError();
	cudaError_t errAsync = cudaDeviceSynchronize();
	if(errSync != cudaSuccess){
        std::cout << "Sync kernel error: " << cudaGetErrorString(errSync) << std::endl;
		rv = 1;
    }
	if(errAsync != cudaSuccess){
        std::cout << "Sync kernel error: " << cudaGetErrorString(errAsync) << std::endl;
		rv = 1;
    }
    
    return rv;
}

DeviceMemoryStats cudaReportMemoryUsage(){
    DeviceMemoryStats memStats;
	cudaError_t status = cudaMemGetInfo(&memStats.free_bytes, &memStats.total_bytes);
    if(status != cudaSuccess){
        std::cout << "Could not query device for memory!" << std::endl;
        memStats.valid = 0;
    }else{
        memStats.used_bytes = memStats.total_bytes - memStats.free_bytes;
        memStats.valid = 1;
    }
    
    return memStats;
}

int cudaHasMemory(size_t bytes){
    DeviceMemoryStats mem = cudaReportMemoryUsage();
    int ok = 0;
    if(mem.valid){
        ok = mem.free_bytes > bytes ? 1 : 0;
    }
    
    return ok;
}

void cudaSafeExit(){
    cudaDeviceReset();
}

void *_cudaAllocate(size_t bytes, int line, const char *filename, bool abort){
    void *ptr = nullptr;
    if(cudaHasMemory(bytes)){
        cudaError_t err = cudaMallocManaged(&ptr, bytes);
        if(err != cudaSuccess){
            std::cout << "Failed to allocate memory " << filename << ":" << line << "[" << bytes << " bytes]" << std::endl;
            ptr = nullptr;
        }else{
            global_memory.allocated += bytes;
        }
    }
    
    if(!ptr && abort){
        getchar();
        cudaSafeExit();
        exit(0);
    }
    
    return ptr;
}