#include <linux/module.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/of_device.h>
#include <linux/pwm.h>
#include <linux/err.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/gpio.h>

#define AI1 20
#define AI2 21
#define BI1 26
#define BI2 16

#define CHK_ERROR(FUNC) { result = FUNC; \
  if (result < 0) { \
    printk(KERN_ERR #FUNC " error in line: %d", __LINE__); \
  } \
}

static struct gpio gpios[] = {
  { AI1, GPIOF_OUT_INIT_LOW,  "AI1" },
  { AI2, GPIOF_OUT_INIT_LOW,  "AI2" },
  { BI1, GPIOF_OUT_INIT_LOW,  "BI1" },
  { BI2, GPIOF_OUT_INIT_LOW,  "BI2" },
};

struct pwm_device* pwmA = NULL;
struct pwm_device* pwmB = NULL;

struct my_data {
    struct device *dev;
    int32_t engine_pwm_value;
};

int result;

static ssize_t engine_power_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    int len;
    struct my_data *my_data = dev_get_drvdata(dev);

    len = sprintf(buf, "%d\n", my_data->engine_pwm_value);
    if (len <= 0)
        dev_err(dev, "mydrv: Invalid sprintf len: %d\n", len);

    return len;
}

static ssize_t engine_power_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
    int32_t power = 0;
    struct my_data *my_data = dev_get_drvdata(dev);

    result = kstrtoint(buf, 10, &my_data->engine_pwm_value);

    if (my_data->engine_pwm_value <=10 && my_data->engine_pwm_value >= -10) {
      gpio_set_value(AI1, 0);
      gpio_set_value(BI1, 0);
      gpio_set_value(AI2, 0);
      gpio_set_value(BI2, 0);
    } else if (my_data->engine_pwm_value < 0) {
      power = (-my_data->engine_pwm_value) + 345;
      gpio_set_value(AI1, 1);
      gpio_set_value(BI1, 1);
      gpio_set_value(AI2, 0);
      gpio_set_value(BI2, 0);
    } else {
      power = (my_data->engine_pwm_value) + 345;
      gpio_set_value(AI1, 0);
      gpio_set_value(BI1, 0);
      gpio_set_value(AI2, 1);
      gpio_set_value(BI2, 1);
    }
    CHK_ERROR(pwm_config(pwmA, (int)power, 1000));
    CHK_ERROR(pwm_config(pwmB, (int)power, 1000));
    CHK_ERROR(pwm_enable(pwmA));
    CHK_ERROR(pwm_enable(pwmB));
    printk(KERN_INFO "Engine power set to value: %d", (int)power);

    return count;
}

static DEVICE_ATTR(engine_power, S_IRUGO | S_IWUSR, engine_power_show,
                   engine_power_store);

static struct attribute *engine_power_attrs[] = {
    &dev_attr_engine_power.attr,
    NULL
};
ATTRIBUTE_GROUPS(engine_power);

static int driver_remove(struct platform_device* pdev) {
  sysfs_remove_group(&pdev->dev.kobj, &engine_power_group);
  return 0;
}

static int driver_probe(struct platform_device* pdev) {
  struct my_data *my_data;

  pwmA = devm_pwm_get(&pdev->dev, "one");
  if (IS_ERR(pwmA)) {
    if ((int)pwmA == -EPROBE_DEFER) {
      printk(KERN_ERR "MOTO EPROBE DEFERED\n");
      return -EPROBE_DEFER;
    } else {
      printk(KERN_ERR "MOTO Error while requesting pwm device, ERR: %d\n", -(int)pwmA);
      return -1;
    }
  }

  pwmB = devm_pwm_get(&pdev->dev, "two");
  if (IS_ERR(pwmB)) {
    if ((int)pwmB == -EPROBE_DEFER) {
      printk(KERN_ERR "MOTO EPROBE DEFERED\n");
      return -EPROBE_DEFER;
    } else {
      printk(KERN_ERR "MOTO Error while requesting pwm device, ERR: %d\n", -(int)pwmB);
      return -1;
    }
  }

  if (gpio_request_array(gpios, ARRAY_SIZE(gpios))) {
      printk(KERN_ERR "MOTO Error while requesting gpio pins\n");
  }


  my_data = devm_kzalloc(&pdev->dev, sizeof(*my_data), GFP_KERNEL);
  my_data->dev = &pdev->dev;
  platform_set_drvdata(pdev, my_data);

  result = sysfs_create_group(&pdev->dev.kobj, &engine_power_group);
  if (result) {
      dev_err(&pdev->dev, "sysfs creation failed\n");
      return result;
  }

  return 0;
}

static const struct of_device_id of_motor_driver_match[] = {
	{ .compatible = "lsroka,motor-controller" },
	{},
};
MODULE_DEVICE_TABLE(of, of_motor_driver_match);

static struct platform_driver motor_platform_driver = {
	.probe	= driver_probe,
	.remove	= driver_remove,
	.driver	= {
		.name	= "motor-driver",
		.of_match_table = of_motor_driver_match
	},
};

module_platform_driver(motor_platform_driver);

MODULE_AUTHOR("Lukasz Sroka <sroka.dev@gmail.com>");
MODULE_DESCRIPTION("PWM Motor Driver");
MODULE_LICENSE("GPL");
