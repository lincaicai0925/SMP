import numpy as np
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec

plt.rcParams['font.sans-serif'] = ['SimHei', 'Microsoft YaHei', 'SimSun']  # Windows中文字体
plt.rcParams['axes.unicode_minus'] = False  # 解决负号显示问题

def generate_realistic_signal(n_points=1000000, seed=42):
    """
    生成包含多种特征的真实信号
    
    公式:
    signal(t) = trend(t) + periodic(t) + noise(t) + spikes(t) + steps(t)
    """
    np.random.seed(seed)
    
    # 归一化时间轴 [0, 1]
    t = np.linspace(0, 1, n_points)
    
    print(f"生成 {n_points:,} 个数据点...")
    
    # ========== 1. 趋势项 ==========
    # 线性上升 + 缓慢的正弦波动
    trend = 5000 * t + 1000 * np.sin(2 * np.pi * 0.5 * t)
    
    # ========== 2. 周期项 ==========
    # 主周期(模拟日周期)
    periodic_main = 2000 * np.sin(2 * np.pi * 10 * t)
    
    # 次周期(模拟小时周期)
    periodic_sub = 500 * np.sin(2 * np.pi * 100 * t)
    
    # 高频周期(模拟分钟级波动)
    periodic_high = 100 * np.sin(2 * np.pi * 500 * t)
    
    periodic = periodic_main + periodic_sub + periodic_high
    
    # ========== 3. 噪声项 ==========
    # 高斯白噪声
    noise = 50 * np.random.randn(n_points)
    
    # ========== 4. 突变项(尖峰) ==========
    spikes = np.zeros(n_points)
    # 随机在0.1%的点上添加尖峰
    spike_indices = np.random.choice(n_points, size=int(n_points * 0.001), replace=False)
    spikes[spike_indices] = np.random.uniform(-1000, 1000, size=len(spike_indices))
    
    # ========== 5. 阶跃变化 ==========
    steps = np.zeros(n_points)
    # 在1/4, 1/2, 3/4处添加阶跃
    step_positions = [n_points // 4, n_points // 2, n_points * 3 // 4]
    step_values = [500, -800, 600]
    
    for pos, val in zip(step_positions, step_values):
        steps[pos:] += val
    
    # ========== 合成最终信号 ==========
    signal = trend + periodic + noise + spikes + steps
    
    # 每隔 1/5 的间隔插入一个较大的值
    interval = n_points // 5
    large_values = [3000, 6000, 9000, 10000]  # 可以自定义幅度

    for i, val in enumerate(large_values, start=1):
        idx = i * interval
        if idx < n_points:
            signal[idx] = val


    
    
    return t, signal, {
        'trend': trend,
        'periodic': periodic,
        'noise': noise,
        'spikes': spikes,
        'steps': steps
    }
    

def plot_signal(t, signal, components, n_points_to_show=10000):
    """
    绘制信号及其组成部分
    """
    # 为了可视化,只显示部分数据
    step = len(t) // n_points_to_show
    t_plot = t[::step]
    signal_plot = signal[::step]
    
    # 创建图形
    fig = plt.figure(figsize=(16, 12))
    gs = GridSpec(4, 2, figure=fig, hspace=0.3, wspace=0.3)
    
    # 1. 完整信号
    ax1 = fig.add_subplot(gs[0, :])
    ax1.plot(t_plot, signal_plot, linewidth=0.5, alpha=0.8)
    ax1.set_title(f'完整信号 (显示 {n_points_to_show:,} / {len(t):,} 个点)', fontsize=14, fontweight='bold')
    ax1.set_xlabel('归一化时间')
    ax1.set_ylabel('信号值')
    ax1.grid(True, alpha=0.3)
    
    # 2. 趋势项
    ax2 = fig.add_subplot(gs[1, 0])
    ax2.plot(t_plot, components['trend'][::step], color='red', linewidth=1)
    ax2.set_title('1. 趋势项: 5000*t + 1000*sin(2π*0.5*t)')
    ax2.set_xlabel('归一化时间')
    ax2.set_ylabel('值')
    ax2.grid(True, alpha=0.3)
    
    # 3. 周期项
    ax3 = fig.add_subplot(gs[1, 1])
    ax3.plot(t_plot, components['periodic'][::step], color='green', linewidth=0.5)
    ax3.set_title('2. 周期项: 多频率叠加')
    ax3.set_xlabel('归一化时间')
    ax3.set_ylabel('值')
    ax3.grid(True, alpha=0.3)
    
    # 4. 噪声项
    ax4 = fig.add_subplot(gs[2, 0])
    ax4.plot(t_plot, components['noise'][::step], color='orange', linewidth=0.3, alpha=0.7)
    ax4.set_title('3. 噪声项: 高斯白噪声')
    ax4.set_xlabel('归一化时间')
    ax4.set_ylabel('值')
    ax4.grid(True, alpha=0.3)
    
    # 5. 尖峰项
    ax5 = fig.add_subplot(gs[2, 1])
    spike_indices = np.where(components['spikes'][::step] != 0)[0]
    if len(spike_indices) > 0:
        ax5.scatter(t_plot[spike_indices], components['spikes'][::step][spike_indices], 
                   color='purple', s=10, alpha=0.6)
    ax5.set_title('4. 突变项: 随机尖峰 (0.1%概率)')
    ax5.set_xlabel('归一化时间')
    ax5.set_ylabel('值')
    ax5.grid(True, alpha=0.3)
    
    # 6. 阶跃项
    ax6 = fig.add_subplot(gs[3, 0])
    ax6.plot(t_plot, components['steps'][::step], color='brown', linewidth=1.5)
    ax6.set_title('5. 阶跃项: 状态切换')
    ax6.set_xlabel('归一化时间')
    ax6.set_ylabel('值')
    ax6.grid(True, alpha=0.3)
    
    # 7. 局部放大(显示细节)
    ax7 = fig.add_subplot(gs[3, 1])
    zoom_start = len(t) // 4
    zoom_end = zoom_start + 1000
    ax7.plot(t[zoom_start:zoom_end], signal[zoom_start:zoom_end], linewidth=1)
    ax7.set_title('局部放大 (1000个点)')
    ax7.set_xlabel('归一化时间')
    ax7.set_ylabel('信号值')
    ax7.grid(True, alpha=0.3)
    
    plt.suptitle('真实复杂信号分解', fontsize=16, fontweight='bold', y=0.995)
    plt.savefig('signal_analysis.png', dpi=150, bbox_inches='tight')
    print("图表已保存为 'signal_analysis.png'")
    plt.show()


def save_data_for_c(signal, filename='test_data.bin', n_samples=1000000):
    """
    将数据保存为二进制文件供C语言读取
    """
    # 转换为int32
    signal_int32 = signal.astype(np.int32)
    
    # 保存为二进制文件
    with open(filename, 'wb') as f:
        f.write(signal_int32.tobytes())
    
    print(f"\n数据已保存到 '{filename}' ({n_samples:,} 个 int32 值, {n_samples*4/1024/1024:.2f} MB)")
    
    # 同时保存一个小样本的文本文件用于验证
    np.savetxt('test_data_sample.txt', signal_int32[:100], fmt='%d')
    print("前100个点已保存到 'test_data_sample.txt' 用于验证")


def print_statistics(signal):
    """
    打印信号统计信息
    """
    print("\n" + "="*60)
    print("信号统计信息:")
    print("="*60)
    print(f"数据点数: {len(signal):,}")
    print(f"最小值:   {np.min(signal):.2f}")
    print(f"最大值:   {np.max(signal):.2f}")
    print(f"平均值:   {np.mean(signal):.2f}")
    print(f"标准差:   {np.std(signal):.2f}")
    print(f"中位数:   {np.median(signal):.2f}")
    print("="*60 + "\n")


if __name__ == "__main__":
    # 生成数据
    n_points = 10000000  # 100万个点
    t, signal, components = generate_realistic_signal(n_points, seed=42)
    
    # 打印统计信息
    print_statistics(signal)
    
    # 保存数据
    save_data_for_c(signal, n_samples=n_points)
    
    # 绘制图形
    print("\n正在生成图表...")
    plot_signal(t, signal, components, n_points_to_show=10000)
    
    print("\n完成!")