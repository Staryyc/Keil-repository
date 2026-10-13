/*************************************************************************
 * 文件名 : twinkle.c
 * 作者   : Staryyc
 * 功能   : 51单片机驱动外部无源蜂鸣器循环播放《小星星》
 * 硬件   : AT89C52 + 12MHz 晶振,蜂鸣器经 8550 三极管接 P2.0
 * 编译   : Keil C51 (本工程已勾选 Output → Create HEX File)
 *
 * 使用步骤:
 *   1. 双击 twinkle.uvprojx 用 Keil 打开本工程
 *   2. 点击工具栏 Build(或按 F7)编译,0 Error 即成功
 *   3. 生成 twinkle.hex 位于工程目录下
 *   4. 烧录到开发板,或在 Proteus 中加载 hex 仿真
 *
 * 修改其他歌曲: 只需改 song[] 音符数组和 beat[] 节拍数组
 *************************************************************************/
#include <reg52.h>

#define uchar unsigned char
#define uint  unsigned int

sbit BEEP = P2^0;              // 蜂鸣器驱动引脚(经三极管驱动)

// ================= 1. 音符频率表(Hz),0=休止 =================
// 简谱 1~7 → C调中高音: do~si
code uint freq[] = {0, 523, 587, 659, 698, 784, 880, 988};
//                  休止  do  re  mi  fa  sol la  si

// ================= 2. 《小星星》曲谱 =================
// 简谱: 1 1 5 5 6 6 5- | 4 4 3 3 2 2 1- | 5 5 4 4 3 3 2-
//       5 5 4 4 3 3 2- | 1 1 5 5 6 6 5- | 4 4 3 3 2 2 1--
code uchar song[] = {
    1,1,5,5,6,6,5,   4,4,3,3,2,2,1,
    5,5,4,4,3,3,2,   5,5,4,4,3,3,2,
    1,1,5,5,6,6,5,   4,4,3,3,2,2,1
};
code uchar beat[] = {          // 与 song 对应的拍数
    1,1,1,1,1,1,2,   1,1,1,1,1,1,2,
    1,1,1,1,1,1,2,   1,1,1,1,1,1,2,
    1,1,1,1,1,1,2,   1,1,1,1,1,1,4
};
#define SONG_LEN (sizeof(song)/sizeof(song[0]))

uint g_val = 0;                // 当前音符的定时器初值

// ================= 3. 延时函数(约 ms 毫秒) =================
void delay_ms(uint ms)
{
    uint i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 115; j++);   // 12MHz 下约为 1ms
}

// ============ 4. 定时器0中断: 半周期翻转电平 → 方波 ============
void timer0_isr(void) interrupt 1
{
    TH0 = (uchar)(g_val >> 8);       // 重装初值
    TL0 = (uchar)(g_val & 0xff);
    BEEP = !BEEP;                    // 翻转电平,产生音符频率
}

// ============ 5. 播放一个音符 ============
// f : 频率(Hz),传 0 表示休止;  ms: 持续时间
void play_tone(uint f, uint ms)
{
    if (f == 0)                      // 休止符:静音等待
    {
        TR0 = 0;
        BEEP = 0;
        delay_ms(ms);
        return;
    }
    /* 12MHz 晶振 12 分频 → 机器周期 1us
       半周期计数值 = 500000/f,初值 = 65536 - 计数值 */
    g_val = (uint)(65536UL - 500000UL / f);
    TH0   = (uchar)(g_val >> 8);
    TL0   = (uchar)(g_val & 0xff);
    TR0   = 1;                       // 启动定时器
    delay_ms(ms);                    // 发音 ms 毫秒
    TR0   = 0;                       // 停止
    BEEP  = 0;
}

// ================= 6. 主函数 =================
void main(void)
{
    uchar i;
    TMOD = 0x01;                     // 定时器0,方式1(16位)
    EA   = 1;                        // 开总中断
    ET0  = 1;                        // 开定时器0中断
    BEEP = 0;

    while (1)                        // 循环播放
    {
        for (i = 0; i < SONG_LEN; i++)
            play_tone(freq[song[i]], beat[i] * 350);  // 一拍 = 350ms(节奏稍快)
        delay_ms(800);               // 曲间停顿
    }
}
