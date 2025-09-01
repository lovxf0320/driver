#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/pwm.h>

int main()
{
    int fd;
    struct pwm_device pwm;
    
    // 打开 pwm 设备
    fd = open("/dev/pwmchip0", O_RDWR);
    if (fd < 0) {
        perror("Failed to open pwmchip device");
        return 1;
    }

    // 配置 PWM 参数
    pwm.channel = 0;
    pwm.duty_cycle = 500000; // 占空比 50%
    pwm.period = 1000000;    // 周期 1ms

    if (ioctl(fd, PWM_IOCTL_SET_DUTY, &pwm) < 0) {
        perror("Failed to set pwm duty cycle");
        close(fd);
        return 1;
    }

    if (ioctl(fd, PWM_IOCTL_SET_PERIOD, &pwm) < 0) {
        perror("Failed to set pwm period");
        close(fd);
        return 1;
    }

    printf("PWM configured: Duty Cycle = %d ns, Period = %d ns\n", pwm.duty_cycle, pwm.period);

    // 关闭设备
    close(fd);

    return 0;
}
