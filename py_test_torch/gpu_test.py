import torch
import time
import matplotlib.pyplot as plt

def test_gpu_performance():
    """测试GPU性能"""
    if not torch.cuda.is_available():
        print("CUDA不可用，使用CPU测试")
        device = torch.device('cpu')
    else:
        device = torch.device('cuda')
        print(f"使用GPU: {torch.cuda.get_device_name()}")
    
    # 矩阵运算性能测试
    sizes = [1000, 2000, 4000, 8000]
    times_gpu = []
    times_cpu = []
    
    for size in sizes:
        print(f"测试矩阵大小: {size}x{size}")
        
        # CPU测试
        a_cpu = torch.randn(size, size)
        b_cpu = torch.randn(size, size)
        
        start_time = time.time()
        c_cpu = torch.matmul(a_cpu, b_cpu)
        cpu_time = time.time() - start_time
        times_cpu.append(cpu_time)
        print(f"  CPU时间: {cpu_time:.4f}秒")
        
        # GPU测试（如果可用）
        if torch.cuda.is_available():
            a_gpu = a_cpu.to(device)
            b_gpu = b_cpu.to(device)
            
            # 预热GPU
            _ = torch.matmul(a_gpu, b_gpu)
            torch.cuda.synchronize()
            
            start_time = time.time()
            c_gpu = torch.matmul(a_gpu, b_gpu)
            torch.cuda.synchronize()
            gpu_time = time.time() - start_time
            times_gpu.append(gpu_time)
            print(f"  GPU时间: {gpu_time:.4f}秒")
            print(f"  加速比: {cpu_time/gpu_time:.2f}x")
        
        print("-" * 40)
    
    # 绘制性能对比图
    plt.figure(figsize=(10, 6))
    plt.plot(sizes, times_cpu, 'o-', label='CPU', linewidth=2)
    if torch.cuda.is_available():
        plt.plot(sizes, times_gpu, 's-', label='GPU', linewidth=2)
    
    plt.xlabel('矩阵大小')
    plt.ylabel('计算时间 (秒)')
    plt.title('CPU vs GPU 矩阵乘法性能对比')
    plt.legend()
    plt.grid(True)
    plt.yscale('log')
    plt.show()

if __name__ == "__main__":
    test_gpu_performance()