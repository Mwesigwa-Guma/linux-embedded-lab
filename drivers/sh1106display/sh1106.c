#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/regmap.h>
#include <linux/property.h>
#include <linux/errno.h>
#include <linux/gpio/consumer.h>
#include <linux/spi/spi.h>

#define DRIVER_NAME             "sh1106"
#define VMEM_SIZE               1056
#define DISPLAY_SMB_REG         0x00
#define SH1106_DEF_FONT_SIZE   5
#define sh1106_MAX_SEG         132

/*
** Variable to store Line Number and Cursor Position.
*/ 
static uint8_t curr_position = 0;
static uint8_t sh1106_font_size  = SH1106_DEF_FONT_SIZE;

struct sh1106_dev{
    struct spi_device *client;
    struct miscdevice mdev;
    struct gpio_desc *dc;
    u8 vmem[VMEM_SIZE];
    char name[10];
    struct regmap *regmap;
};

static const struct regmap_config sh1106_regmap_config = {
    .reg_bits = 8,
    .val_bits = 8,
    // Configure additional regmap settings as needed
};


static const uint8_t sh1106_init_seq[] = {
    0xAE,             // Display OFF
    0xD5, 0x80,       // Set display clock divide ratio/oscillator frequency
    0xA8, 0x3F,       // Set multiplex ratio (0x3F = 64)
    0xD3, 0x00,       // Set display offset = 0
    0x40,             // Set display start line = 0
    0xAD, 0x8B,       // Enable internal DC-DC
    0xA1,             // Set segment re-map (mirror horizontally)
    0xC8,             // Set COM output scan direction (mirror vertically)
    0xDA, 0x12,       // Set COM pins hardware configuration
    0x81, 0xCF,       // Set contrast
    0xD9, 0xF1,       // Set pre-charge period
    0xDB, 0x40,       // Set VCOMH deselect level
    0xA4,             // Resume to RAM content display
    0xA6,             // Normal display mode (not inverse)
    0xAF              // Display ON
};

/*
** Array Variable to store the letters.
*/ 
static const unsigned char sh1106_font[][SH1106_DEF_FONT_SIZE]= 
{
    {0x00, 0x00, 0x00, 0x00, 0x00},   // space
    {0x00, 0x00, 0x2f, 0x00, 0x00},   // !
    {0x00, 0x07, 0x00, 0x07, 0x00},   // "
    {0x14, 0x7f, 0x14, 0x7f, 0x14},   // #
    {0x24, 0x2a, 0x7f, 0x2a, 0x12},   // $
    {0x23, 0x13, 0x08, 0x64, 0x62},   // %
    {0x36, 0x49, 0x55, 0x22, 0x50},   // &
    {0x00, 0x05, 0x03, 0x00, 0x00},   // '
    {0x00, 0x1c, 0x22, 0x41, 0x00},   // (
    {0x00, 0x41, 0x22, 0x1c, 0x00},   // )
    {0x14, 0x08, 0x3E, 0x08, 0x14},   // *
    {0x08, 0x08, 0x3E, 0x08, 0x08},   // +
    {0x00, 0x00, 0xA0, 0x60, 0x00},   // ,
    {0x08, 0x08, 0x08, 0x08, 0x08},   // -
    {0x00, 0x60, 0x60, 0x00, 0x00},   // .
    {0x20, 0x10, 0x08, 0x04, 0x02},   // /

    {0x3E, 0x51, 0x49, 0x45, 0x3E},   // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00},   // 1
    {0x42, 0x61, 0x51, 0x49, 0x46},   // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31},   // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10},   // 4
    {0x27, 0x45, 0x45, 0x45, 0x39},   // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30},   // 6
    {0x01, 0x71, 0x09, 0x05, 0x03},   // 7
    {0x36, 0x49, 0x49, 0x49, 0x36},   // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E},   // 9

    {0x00, 0x36, 0x36, 0x00, 0x00},   // :
    {0x00, 0x56, 0x36, 0x00, 0x00},   // ;
    {0x08, 0x14, 0x22, 0x41, 0x00},   // <
    {0x14, 0x14, 0x14, 0x14, 0x14},   // =
    {0x00, 0x41, 0x22, 0x14, 0x08},   // >
    {0x02, 0x01, 0x51, 0x09, 0x06},   // ?
    {0x32, 0x49, 0x59, 0x51, 0x3E},   // @

    {0x7C, 0x12, 0x11, 0x12, 0x7C},   // A
    {0x7F, 0x49, 0x49, 0x49, 0x36},   // B
    {0x3E, 0x41, 0x41, 0x41, 0x22},   // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C},   // D
    {0x7F, 0x49, 0x49, 0x49, 0x41},   // E
    {0x7F, 0x09, 0x09, 0x09, 0x01},   // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A},   // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F},   // H
    {0x00, 0x41, 0x7F, 0x41, 0x00},   // I
    {0x20, 0x40, 0x41, 0x3F, 0x01},   // J
    {0x7F, 0x08, 0x14, 0x22, 0x41},   // K
    {0x7F, 0x40, 0x40, 0x40, 0x40},   // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F},   // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F},   // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E},   // O
    {0x7F, 0x09, 0x09, 0x09, 0x06},   // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E},   // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46},   // R
    {0x46, 0x49, 0x49, 0x49, 0x31},   // S
    {0x01, 0x01, 0x7F, 0x01, 0x01},   // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F},   // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F},   // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F},   // W
    {0x63, 0x14, 0x08, 0x14, 0x63},   // X
    {0x07, 0x08, 0x70, 0x08, 0x07},   // Y
    {0x61, 0x51, 0x49, 0x45, 0x43},   // Z

    {0x00, 0x7F, 0x41, 0x41, 0x00},   // [
    {0x55, 0xAA, 0x55, 0xAA, 0x55},   // Backslash (Checker pattern)
    {0x00, 0x41, 0x41, 0x7F, 0x00},   // ]
    {0x04, 0x02, 0x01, 0x02, 0x04},   // ^
    {0x40, 0x40, 0x40, 0x40, 0x40},   // _
    {0x00, 0x03, 0x05, 0x00, 0x00},   // `

    {0x20, 0x54, 0x54, 0x54, 0x78},   // a
    {0x7F, 0x48, 0x44, 0x44, 0x38},   // b
    {0x38, 0x44, 0x44, 0x44, 0x20},   // c
    {0x38, 0x44, 0x44, 0x48, 0x7F},   // d
    {0x38, 0x54, 0x54, 0x54, 0x18},   // e
    {0x08, 0x7E, 0x09, 0x01, 0x02},   // f
    {0x18, 0xA4, 0xA4, 0xA4, 0x7C},   // g
    {0x7F, 0x08, 0x04, 0x04, 0x78},   // h
    {0x00, 0x44, 0x7D, 0x40, 0x00},   // i
    {0x40, 0x80, 0x84, 0x7D, 0x00},   // j
    {0x7F, 0x10, 0x28, 0x44, 0x00},   // k
    {0x00, 0x41, 0x7F, 0x40, 0x00},   // l
    {0x7C, 0x04, 0x18, 0x04, 0x78},   // m
    {0x7C, 0x08, 0x04, 0x04, 0x78},   // n
    {0x38, 0x44, 0x44, 0x44, 0x38},   // o
    {0xFC, 0x24, 0x24, 0x24, 0x18},   // p
    {0x18, 0x24, 0x24, 0x18, 0xFC},   // q
    {0x7C, 0x08, 0x04, 0x04, 0x08},   // r
    {0x48, 0x54, 0x54, 0x54, 0x20},   // s
    {0x04, 0x3F, 0x44, 0x40, 0x20},   // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C},   // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C},   // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C},   // w
    {0x44, 0x28, 0x10, 0x28, 0x44},   // x
    {0x1C, 0xA0, 0xA0, 0xA0, 0x7C},   // y
    {0x44, 0x64, 0x54, 0x4C, 0x44},   // z

    {0x00, 0x10, 0x7C, 0x82, 0x00},   // {
    {0x00, 0x00, 0xFF, 0x00, 0x00},   // |
    {0x00, 0x82, 0x7C, 0x10, 0x00},   // }
    {0x00, 0x06, 0x09, 0x09, 0x06}    // ~ (Degrees)
};

static int sh1106_open(struct inode *inode, struct file *file)
{
    struct miscdevice *misc = file->private_data;
    struct sh1106_dev *sh1106 = container_of(misc, struct sh1106_dev, mdev);

    if (!sh1106) {
        pr_err("sh1106: Failed to retrieve private data in open\n");
        return -ENODEV;
    }

    file->private_data = sh1106; // Set private data for subsequent operations
    dev_info(&sh1106->client->dev, "sh1106 device opened\n");

    file->private_data = sh1106;
    return 0;
}

static int sh1106_send_commands(struct sh1106_dev *display, const uint8_t *buf, size_t len) {
    int ret;

    if (!display->dc) {
        dev_err(&display->client->dev, "D/C GPIO not initialized\n");
        return -ENODEV;
    }

    gpiod_set_value_cansleep(display->dc, 0);  // D/C = 0 (command)
    ret = regmap_bulk_write(display->regmap, DISPLAY_SMB_REG, buf, len);

    return ret;
}

static int sh1106_send_data(struct sh1106_dev *display, const uint8_t *buf, size_t len) {
    int ret;

    if (!display->dc) {
        dev_err(&display->client->dev, "D/C GPIO not initialized\n");
        return -ENODEV;
    }

    gpiod_set_value_cansleep(display->dc, 1);  // D/C = 1 (data)
    ret = regmap_bulk_write(display->regmap, DISPLAY_SMB_REG, buf, len);

    return ret;
}

static int sh1106_update_display(struct sh1106_dev *display){
    if (!display || !display->client) {
        pr_err("sh1106: Invalid display or client pointer in update_display\n");
        return -ENODEV;
    }
		
    int ret = sh1106_send_data(display, display->vmem, VMEM_SIZE);
    if(ret < 0)
        dev_err(&display->client->dev, "update sh1106 OLED Display failed");

    return ret;
}

static void sh1106_update_buffer_char(struct sh1106_dev *display, unsigned char c){
    uint8_t data_byte;
    uint8_t temp = 0;

    // print charcters other than new line
    if( c != '\n' )
    {
        /*
        ** In our font array (SSD1306_font), space starts in 0th index.
        ** But in ASCII table, Space starts from 32 (0x20).
        ** So we need to match the ASCII table with our font table.
        ** We can subtract 32 (0x20) in order to match with our font table.
        */
        c -= 0x20;  //or c -= ' ';

        do
        {
            data_byte= sh1106_font[c][temp]; // Get the data to be displayed from LookUptable

            display->vmem[curr_position] = data_byte; // write data to the OLED
            curr_position++;
            temp++;
        
        } while ( temp < sh1106_font_size);
        display->vmem[curr_position] = 0x00;         //Display the data
        curr_position++;
    }
}

// File operations: write function
static ssize_t sh1106_write(struct file *file, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct sh1106_dev *sh1106 = file->private_data;
    if(!sh1106 || !sh1106->client){
    	pr_err("sh1106: invalid device pointer");
	return -ENODEV;
    }
    int ret;
    int i;
    uint8_t kbuf[176];
    curr_position = 0;

    if (!sh1106->dc) {
        dev_err(&sh1106->client->dev, "D/C GPIO not initialized\n");
        return -ENODEV;
    }

    if(copy_from_user(kbuf, buf, count))
        return -EFAULT;

    for(i = 0; i < count; i++){
        sh1106_update_buffer_char(sh1106, kbuf[i]);
    }
    
    ret = sh1106_update_display(sh1106);
    return ret;
}

// File operations structure
static const struct file_operations oled_fops = {
    .owner = THIS_MODULE,
    .open = sh1106_open,
    .write = sh1106_write,
};

static int sh1106_probe(struct spi_device *client)
{
    dev_info(&client->dev, "sh1106 oled display driver probing\n");

    struct sh1106_dev * sh1106;
    int ret;

    // Allocate private structure 
    sh1106 = devm_kzalloc(&client->dev, sizeof(struct sh1106_dev), GFP_KERNEL);
    if(!sh1106){
        dev_err(&client->dev, "Failed to allocate memory for sh1106\n");
        return -ENOMEM;
    }

    sh1106->client = client;

    // Initialize the regmap
    sh1106->regmap = devm_regmap_init_spi(client, &sh1106_regmap_config);
    if (IS_ERR(sh1106->regmap)) {
        dev_err(&client->dev, "Failed to initialize regmap\n");
        return PTR_ERR(sh1106->regmap);
    }

    // Get the D/C GPIO
    sh1106->dc = devm_gpiod_get(&client->dev, "dc", GPIOD_OUT_LOW);
    if (IS_ERR(sh1106->dc)) {
        dev_err(&client->dev, "Failed to get D/C GPIO\n");
        return PTR_ERR(sh1106->dc);
    }

    spi_set_drvdata(client, sh1106);

    // Initialize the misc device
    strncpy(sh1106->name, "sh1106", sizeof(sh1106->name) - 1);

    ret = sh1106_send_commands(sh1106, sh1106_init_seq, sizeof(sh1106_init_seq));
    if (ret) {
        dev_err(&client->dev, "SH1106 initialization sequence failed: %d\n", ret);
        return ret;
    }

    memset(sh1106->vmem, 0, VMEM_SIZE);
    sh1106_update_display(sh1106);

    sh1106->mdev = (struct miscdevice){
        .minor = MISC_DYNAMIC_MINOR,
        .name = "sh1106",
        .mode = 0666,
        .fops = &oled_fops,
	.parent = &client->dev,
	.this_device = &client->dev,
    };

    dev_set_drvdata(&client->dev, sh1106);

    ret = misc_register(&sh1106->mdev);
    if(ret < 0){
        dev_err(&client->dev, "SH1106 misc device registration failed\n");
        return ret;
    }

    dev_info(&client->dev, "sh1106 display registered with minor number %i\n", sh1106->mdev.minor);
    return 0;
}

static void sh1106_remove(struct spi_device *client)
{
    // Get device structure from device bus context
    struct sh1106_dev *sh1106 = spi_get_drvdata(client);
    
    // Deregister misc device
    misc_deregister(&sh1106->mdev);

    dev_info(&client->dev,  "Exiting remove function for sh1106 driver");
}

static const struct of_device_id oled_id[] = {
    { .compatible = "sino,sh1106" },
    { }
};
MODULE_DEVICE_TABLE(of, oled_id);

static struct spi_device_id sh1106_id[] = {
    {"sh1106", 0},
    { },
};
MODULE_DEVICE_TABLE(spi, sh1106_id);

static struct spi_driver sh1106_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = oled_id,
    },
    .id_table = sh1106_id,
    .probe = sh1106_probe,
    .remove = sh1106_remove,
};

module_spi_driver(sh1106_driver);

MODULE_AUTHOR("Mwesigwa Guma");
MODULE_DESCRIPTION("Platform Driver for SINO sh1106 OLED Display");
MODULE_LICENSE("GPL");
