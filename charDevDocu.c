#include <linux/atomic.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h> /* para sprintf() */
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/uaccess.h> /* para get_user y put_user */

#include <asm/errno.h>

/* Prototipos - esto normalmente iría en un archivo .h */
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev" /* Nombre del dispositivo tal como aparece en /proc/devices */
#define BUF_LEN 80 /* Longitud máxima del mensaje desde el dispositivo */


/* Las variables globales se declaran como estáticas, por lo que son globales dentro del archivo. */

static int major; /* Número mayor asignado a nuestro controlador de dispositivo */

enum {
    CDEV_NOT_USED = 0,
    CDEV_EXCLUSIVE_OPEN = 1,
};

/* ¿Está abierto el dispositivo? Se usa para prevenir el acceso múltiple al dispositivo */
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED);

/* El mensaje que el dispositivo dará cuando se le pregunte */
static char msg[BUF_LEN + 1]; 

static struct class *cls;

static struct file_operations chardev_fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
};

static int __init chardev_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &chardev_fops);

    if (major < 0) {
        pr_alert("Error al registrar el dispositivo de caracteres con %d\n", major);
        return major;
    }

    pr_info("Se me asignó el número mayor %d.\n", major);

    cls = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);

    pr_info("Dispositivo creado en /dev/%s\n", DEVICE_NAME);

    return SUCCESS;
}

static void __exit chardev_exit(void)
{
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);

    /* Deregistrar el dispositivo */
    unregister_chrdev(major, DEVICE_NAME);
}

/* Métodos */

/* Llamada cuando un proceso intenta abrir el archivo del dispositivo, como
 * "sudo cat /dev/chardev"
 */
static int device_open(struct inode *inode, struct file *file)
{
    static int counter = 0;

    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN))
        return -EBUSY;

   
    try_module_get(THIS_MODULE);

    return SUCCESS;
}

/* Llamada cuando un proceso cierra el archivo del dispositivo */
static int device_release(struct inode *inode, struct file *file)
{
    /* Ahora estamos listos para nuestro próximo llamador */
    atomic_set(&already_open, CDEV_NOT_USED);

    /* Decrementar el contador de uso, o de lo contrario una vez que hayas abierto el archivo, nunca te desharás del módulo */
    module_put(THIS_MODULE);

    return SUCCESS;
}

/* Llamada cuando un proceso, que ya abrió el archivo del dispositivo, intenta leer de él */
static ssize_t device_read(struct file *filp,
                           char __user *buffer,
                           size_t length,
                           loff_t *offset)
{
    /* Número de bytes realmente escritos en el búfer */
    int bytes_read = 0;
    const char *msg_ptr = msg;
    ssize_t msg_len = strlen(msg);

    /* Si estamos al final del mensaje, retornar 0 (EOF) */
    if (*offset >= msg_len) {
        return 0;
    }

    /* Poner realmente los datos en el búfer */
    while (length && (*offset < msg_len)) {
        /* El búfer está en el segmento de datos de usuario, no en el segmento de kernel,
         * por lo que la asignación "*" no funcionará. Tenemos que usar put_user que copia
         * datos del segmento de datos del kernel al segmento de datos del usuario.
         */
        put_user(*(msg_ptr + msg_len - 1 - *offset), buffer++);
        (*offset)++;
        length--;
        bytes_read++;
    }

    /* La mayoría de las funciones de lectura devuelven el número de bytes colocados en el búfer. */
    return bytes_read;
}

/* Llamada cuando un proceso escribe en el archivo del dispositivo */
static ssize_t device_write(struct file *filp, const char __user *buff,
                            size_t len, loff_t *off)
{
    // Verificar si el tamaño del mensaje excede la longitud máxima permitida
    if (len > BUF_LEN) {
        pr_alert("Longitud del mensaje excede el límite.\n");
        return -EINVAL;
    }

    // Copiar el mensaje desde el espacio de usuario al espacio del kernel
    if (copy_from_user(msg, buff, len) != 0) {
        pr_alert("Error al copiar desde el espacio de usuario al espacio del kernel.\n");
        return -EFAULT;  // Error de copia
    }

    // Agregar el carácter nulo al final del mensaje para asegurar que sea una cadena válida
    msg[len] = '\0';

    // Imprimir el mensaje en el kernel
    pr_info("Mensaje recibido en el dispositivo: %s\n", msg);

    // Devolver el número de bytes escritos (en este caso, simplemente la longitud del mensaje)
    return len;
}




module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
