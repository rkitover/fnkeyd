#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

static volatile sig_atomic_t stop = 0;

static void interrupt_handler(int sig)
{
        stop = 1;
}

int emit(struct libevdev_uinput *uidev, unsigned int type, unsigned int code, unsigned int value)
{
        int rc = 1;

        rc = libevdev_uinput_write_event(uidev, type, code, value);
        if (rc != 0)
                return rc;

        return libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
}

int main(int argc, char** argv)
{
        struct libevdev *dev = NULL;
        struct libevdev_uinput *uidev = NULL;
        int fd = -1;
        int rc = 1;
        int ev_rc = 0;
        unsigned int down = 1;

        if (getuid() != 0) {
                fprintf(stderr, "Must be root.\n");
                return EXIT_FAILURE;
        }

        if (argc != 2) {
                fprintf(stderr, "Usage:\n\n%s /dev/input/event<X>\n", argv[0]);
                return EXIT_FAILURE;
        }

        fd = open(argv[1], O_RDONLY|O_NONBLOCK);
        if (fd < 0) {
                fprintf(stderr, "Failed to open %s: %s\n", argv[1], strerror(-fd));
                return EXIT_FAILURE;
        }

        rc = libevdev_new_from_fd(fd, &dev);
        if (rc < 0) {
                fprintf(stderr, "Failed to init libevdev: %s\n", strerror(-rc));
                goto error;
        }

        rc = libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);
        if (rc != 0) {
                fprintf(stderr, "Failed to init uinput: %s\n", strerror(-rc));
                goto error;
        }

        signal(SIGINT, interrupt_handler);
        signal(SIGTERM, interrupt_handler);

        do {
                struct input_event ev;

                ev_rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);

                if (ev_rc == 0 && ev.type == EV_MSC && ev.code == MSC_SCAN && (ev.value == 0xf8 || ev.value == 0xe3)) {
                        rc = emit(uidev, EV_KEY, KEY_LEFTSHIFT, down);
                        if (rc != 0)
                                goto error;

                        rc = emit(uidev, EV_KEY, KEY_LEFTCTRL, down);
                        if (rc != 0)
                                goto error;

                        rc = emit(uidev, EV_KEY, KEY_LEFTMETA, down);
                        if (rc != 0)
                                goto error;

                        rc = emit(uidev, EV_KEY, KEY_LEFTALT, down);
                        if (rc != 0)
                                goto error;

                        down = !down;
                }
        } while (!stop && (ev_rc == 1 || ev_rc == 0 || ev_rc == -EAGAIN));

        rc = EXIT_SUCCESS;

error:
        libevdev_uinput_destroy(uidev);
        libevdev_free(dev);
        close(fd);

        return rc;
}
