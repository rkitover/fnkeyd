#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

int main(int argc, char** argv)
{
        struct libevdev *dev = NULL;
        struct libevdev_uinput *uidev = NULL;
        int fd, uifd;
        int rc = 1;
        unsigned int down = 1;

        fd = open("/dev/input/event0", O_RDONLY|O_NONBLOCK);
        rc = libevdev_new_from_fd(fd, &dev);
        if (rc < 0) {
                fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-rc));
                exit(1);
        }
        uifd = open("/dev/uinput", O_RDWR);
        if (uifd < 0)
                return -errno;

        rc = libevdev_uinput_create_from_device(dev, uifd, &uidev);
        if (rc != 0)
                return rc;

        do {
                struct input_event ev;
                rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
                if (rc == 0 && strcmp(libevdev_event_type_get_name(ev.type), "EV_MSC") == 0
                            && strcmp(libevdev_event_code_get_name(ev.type, ev.code), "MSC_SCAN") == 0
                            && ev.value == 0xf8) {

                        rc = libevdev_uinput_write_event(uidev, EV_KEY, KEY_LEFTSHIFT, down);
                        if (rc != 0)
                                return rc;

                        rc = libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
                        if (rc != 0)
                                return rc;

                        rc = libevdev_uinput_write_event(uidev, EV_KEY, KEY_LEFTCTRL, down);
                        if (rc != 0)
                                return rc;

                        rc = libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
                        if (rc != 0)
                                return rc;

                        rc = libevdev_uinput_write_event(uidev, EV_KEY, KEY_LEFTMETA, down);
                        if (rc != 0)
                                return rc;

                        rc = libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
                        if (rc != 0)
                                return rc;

                        rc = libevdev_uinput_write_event(uidev, EV_KEY, KEY_LEFTALT, down);
                        if (rc != 0)
                                return rc;

                        rc = libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
                        if (rc != 0)
                                return rc;

                        down = !down;
                }
        } while (rc == 1 || rc == 0 || rc == -EAGAIN);

        libevdev_uinput_destroy(uidev);
        close(uifd);
        libevdev_free(dev);
        close(fd);

        return EXIT_SUCCESS;
}
