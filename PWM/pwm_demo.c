#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pwm.h>
#include <linux/platform_device.h>
#include <linux/of.h>

#define MAX_PWM_CHANNELS 1

struct pwm_demo {
    struct pwm_chip chip;
    struct pwm_device *pwm;
};

static int pwm_demo_config(struct pwm_chip *chip, struct pwm_device *pwm,
                           int duty_ns, int period_ns)
{
    // 这里可以进行实际的硬件配置
    printk(KERN_INFO "Configuring PWM: duty=%d ns, period=%d ns\n", duty_ns, period_ns);
    return 0;
}

static const struct pwm_ops pwm_demo_ops = {
    .config = pwm_demo_config,
    .apply = NULL,  // 这里可以实现实际的应用操作
};

static int pwm_demo_probe(struct platform_device *pdev)
{
    struct pwm_demo *demo;
    struct device_node *node = pdev->dev.of_node;
    int ret;

    demo = devm_kzalloc(&pdev->dev, sizeof(*demo), GFP_KERNEL);
    if (!demo)
        return -ENOMEM;

    platform_set_drvdata(pdev, demo);

    demo->chip.ops = &pwm_demo_ops;
    demo->chip.npwm = MAX_PWM_CHANNELS;

    ret = pwmchip_add(&demo->chip);
    if (ret < 0) {
        dev_err(&pdev->dev, "Failed to add pwm chip\n");
        return ret;
    }

    demo->pwm = pwm_get(&pdev->dev, 0);
    if (IS_ERR(demo->pwm)) {
        dev_err(&pdev->dev, "Failed to get pwm device\n");
        return PTR_ERR(demo->pwm);
    }

    return 0;
}

static int pwm_demo_remove(struct platform_device *pdev)
{
    struct pwm_demo *demo = platform_get_drvdata(pdev);

    pwmchip_remove(&demo->chip);

    return 0;
}

static const struct of_device_id pwm_demo_of_match[] = {
    { .compatible = "my,pwm-demo", },
    {},
};

MODULE_DEVICE_TABLE(of, pwm_demo_of_match);

static struct platform_driver pwm_demo_driver = {
    .probe = pwm_demo_probe,
    .remove = pwm_demo_remove,
    .driver = {
        .name = "pwm_demo",
        .of_match_table = pwm_demo_of_match,
    },
};

module_platform_driver(pwm_demo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple PWM demo driver");
