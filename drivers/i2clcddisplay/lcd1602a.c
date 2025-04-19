// lcd1602_driver.c

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/ioctl.h>

#define DRIVER_NAME "lcd1602"

// Example LCD I2C address (can vary)
#define LCD_I2C_ADDR 0x3f
#define LCD_BACKLIGHT 0x08
#define ENABLE 0x04
#define LCD_RS 0x01

struct lcd1602_dev{
    struct i2c_client *lcd_client;
    struct miscdevice lcd1602_miscdev;
    char name[10];
};

// Function to send a command to the LCD
static void lcd_send_cmd(struct i2c_client *client, uint8_t cmd)
{
    uint8_t data_u = (cmd & 0xF0) | LCD_BACKLIGHT;
    uint8_t data_l = ((cmd << 4) & 0xF0) | LCD_BACKLIGHT;

    uint8_t data_arr[4] = {
        data_u | ENABLE,
        data_u,
        data_l | ENABLE,
        data_l
    };

    for (int i = 0; i < 4; i++) {
        int ret = i2c_smbus_write_byte(client, data_arr[i]);
        if (ret < 0) {
            dev_err(&client->dev, "i2c_smbus_write_byte failed: %d\n", ret);
            return;
        }
        msleep(1);
    }

    msleep(2);
}

// Function to send data to the LCD
static void lcd_send_data(struct i2c_client *client, uint8_t data)
{
    uint8_t data_u = (data & 0xF0) | LCD_BACKLIGHT | ENABLE | LCD_RS;
    uint8_t data_l = ((data << 4) & 0xF0) | LCD_BACKLIGHT | ENABLE | LCD_RS;

    uint8_t data_arr[4] = {
        data_u,
        data_u & ~ENABLE,
        data_l,
        data_l & ~ENABLE
    };

    for (int i = 0; i < 4; i++) {
        int ret = i2c_smbus_write_byte(client, data_arr[i]);
        if (ret < 0) {
            dev_err(&client->dev, "i2c_smbus_write_byte failed: %d\n", ret);
            return;
        }
        msleep(1);
    }

    msleep(2);
}

// File operations: write function
static ssize_t lcd1602_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char kbuf[32];
    size_t i;
    struct lcd1602_dev *lcd1602 = file->private_data;
    static uint8_t cursor_position = 0x80; // Default to first line

    if (count > sizeof(kbuf) - 1)
        count = sizeof(kbuf) - 1;

    if (copy_from_user(kbuf, buf, count))
        return -EFAULT;

    kbuf[count] = '\0';
    
    if (!lcd1602 || !lcd1602->lcd_client)
        return -ENODEV;

    // Clear display and set cursor to the current position
    lcd_send_cmd(lcd1602->lcd_client, 0x01);
    lcd_send_cmd(lcd1602->lcd_client, cursor_position);

    for (i = 0; i < count; i++) {
        if (i == 16 && cursor_position == 0x80) {
            lcd_send_cmd(lcd1602->lcd_client, 0xC0); // Wrap to second line
        } else if (i == 16 && cursor_position == 0xC0) {
            break; // Stop writing if second line is full
        }

        dev_info(&lcd1602->lcd_client->dev, "lcd1602: sending char: 0x%02x (%c)\n", kbuf[i], kbuf[i]);
        lcd_send_data(lcd1602->lcd_client, kbuf[i]);
    }

    return count;
}

static int lcd1602_open(struct inode *inode, struct file *file)
{
    struct miscdevice *misc = file->private_data;
    struct lcd1602_dev *lcd1602 = container_of(misc, struct lcd1602_dev, lcd1602_miscdev);
    file->private_data = lcd1602;
    return 0;
}

static int lcd1602_release(struct inode *inode, struct file *file)
{
    struct lcd1602_dev *lcd1602 = file->private_data;
    dev_info(&lcd1602->lcd_client->dev, "lcd1602: device closed for lcd1602");
    return 0;
}

static long lcd1602_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct lcd1602_dev *lcd1602 = file->private_data;
    static uint8_t cursor_position = 0x80; // Default to first line

    switch (cmd) {
        case 0x01: // Clear display
            lcd_send_cmd(lcd1602->lcd_client, 0x01);
            lcd_send_cmd(lcd1602->lcd_client, 0x80);
            cursor_position = 0x80; // Reset to first line
            return 0;
        case 0x80: // Set cursor to first line position 0
            lcd_send_cmd(lcd1602->lcd_client, 0x80);
            cursor_position = 0x80;
            return 0;
        case 0xC0: // Set cursor to second line position 0
            lcd_send_cmd(lcd1602->lcd_client, 0xC0);
	    dev_info(&lcd1602->lcd_client->dev, "change cursor position");
            cursor_position = 0xC0;
            return 0;
        default:
            return -EINVAL;
    }
}

// File operations structure
static const struct file_operations lcd_fops = {
    .owner = THIS_MODULE,
    .open = lcd1602_open,
    .release = lcd1602_release,
    .unlocked_ioctl = lcd1602_ioctl,
    .write = lcd1602_write,
};

static void initialize_lcd(struct i2c_client *client){
    msleep(50);
    lcd_send_cmd(client, 0x33); // Initialize
    msleep(5);
    lcd_send_cmd(client, 0x32); // Set to 4-bit mode
    msleep(5);
    lcd_send_cmd(client, 0x28); // 2 line, 5x7 matrix
    msleep(5);
    lcd_send_cmd(client, 0x0C); // Display on, cursor off
    msleep(5);
    lcd_send_cmd(client, 0x06); // Increment cursor
    msleep(5);
    lcd_send_cmd(client, 0x01); // Clear display
    msleep(5);
    lcd_send_cmd(client, 0x80);  // Move cursor to line 1, pos 0
    msleep(5);
    
    dev_info(&client->dev, "lcd1602: Initialization complete\n");
}

static int lcd1602_probe(struct i2c_client *client)
{
    dev_info(&client->dev, "LCD 1602 I2C driver probed\n");

    int counter = 0;
    struct lcd1602_dev * lcd1602;

    // Allocate private structure 
    lcd1602 = devm_kzalloc(&client->dev, sizeof(struct lcd1602_dev), GFP_KERNEL);
    if(!lcd1602){
        dev_info(&client->dev, "lcd1602 unable to allocate memory");
        return -ENOMEM;
    }

    // store pointer to the device structure in bus device context
    i2c_set_clientdata(client, lcd1602);

    // store pointer to I2C client
    lcd1602->lcd_client = client;

    // initialize the misc device, lcd1602 is incremented after each probe call
    sprintf(lcd1602->name, "lcd1602%02d", counter++);

    lcd1602->lcd1602_miscdev.name = lcd1602->name;
    lcd1602->lcd1602_miscdev.minor = MISC_DYNAMIC_MINOR;
    lcd1602->lcd1602_miscdev.fops = &lcd_fops;
    lcd1602->lcd1602_miscdev.parent = &client->dev;

    int ret = misc_register(&lcd1602->lcd1602_miscdev);
    if(ret < 0){
        dev_info(&client->dev, "device registration failed");
        return ret;
    }

    initialize_lcd(client);

    return 0;
}

static void lcd1602_remove(struct i2c_client *client)
{
    struct lcd1602_dev * lcd1602;

    // Get device structure from device bus context
    lcd1602 = i2c_get_clientdata(client);

    dev_info(&client->dev, "entered remove function for lcd1602");

    // Deregister misc device
    misc_deregister(&lcd1602->lcd1602_miscdev);

    dev_info(&client->dev,  "Exiting remove function for lcd1602");
}

static const struct of_device_id lcd_ids[] = {
    { .compatible = "qapass,lcd1602" },
    { }
};
MODULE_DEVICE_TABLE(of, lcd_ids);

static const struct i2c_device_id lcd1602_id[] = {
    { .name = "lcd1602"},
    { }
};
MODULE_DEVICE_TABLE(i2c, lcd1602_id);

static struct i2c_driver lcd1602_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
        .of_match_table = lcd_ids,
    },
    .probe = lcd1602_probe,
    .remove = lcd1602_remove,
    .id_table = lcd1602_id,
};

module_i2c_driver(lcd1602_driver);

MODULE_AUTHOR("Mwesigwa Guma");
MODULE_DESCRIPTION("I2C Driver for QAPASS 1602A LCD");
MODULE_LICENSE("GPL");
