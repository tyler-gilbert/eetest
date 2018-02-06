
#include <cstdio>
#include <sos/dev/eeprom.h>
#include <sapi/sys.hpp>
#include <sapi/hal.hpp>

static int read_write_test(Device & eeprom, int page_size, const eeprom_info_t & info);
static void erase_eeprom(Device & eeprom, const eeprom_info_t & info);

int main(int argc, char * argv[]){
	Cli cli(argc, argv);
	cli.set_publisher("Stratify Labs, Inc");
	cli.handle_version();

    Device eeprom;
    eeprom_info_t info;

    printf("Open EEPROM\n");
    if( eeprom.open("/dev/eeprom0", Device::RDWR) < 0 ){
        printf("Failed to open EEPROM\n");
    } else {

        if( eeprom.ioctl(I_EEPROM_GETINFO, &info) < 0 ){
            printf("failed to get EEPROM info\n");
        } else {
            read_write_test(eeprom, 30, info);
            read_write_test(eeprom, 60, info);
            read_write_test(eeprom, 80, info);
            read_write_test(eeprom, 120, info);
            read_write_test(eeprom, 256, info);
            erase_eeprom(eeprom, info);
        }


        eeprom.close();
    }


	return 0;
}

int read_write_test(Device & eeprom, int page_size, const eeprom_info_t & info){
    int i;
    int j;
    char buffer[page_size];

    printf("Start r/w %d\n", page_size);


    for(i=0; i < page_size; i++){
        buffer[i] = i;
    }


    for(j=0; j + page_size < (int)info.size; j+=page_size){
        printf("Write %d of %ld -- %d\n", j, info.size, page_size);
        if( eeprom.write(j, buffer, page_size) != page_size ){
            printf("Failed to write eeprom at %d (%d bytes)--%d\n", j, page_size, errno);
            return -1;
        }
    }

    for(j=0; j + page_size < (int)info.size; j+=page_size){
        memset(buffer, 0, page_size);
        printf("Read %d of %ld -- %d\n", j, info.size, page_size);
        if( eeprom.read(j, buffer, page_size) != page_size ){
            printf("Failed to write eeprom at %d (%d bytes)--%d\n", j, page_size, errno);
            exit(1);
        }

        for(i=0; i < page_size; i++){
            if( buffer[i] != i ){
                printf("Failed to verify EEPROM at %d for %d bytes\n", j+i, page_size);
                exit(1);
            }
        }
    }
    return 0;
}

void erase_eeprom(Device & eeprom, const eeprom_info_t & info){
    u32 i, j;
    u8 buffer[info.page_size];

    printf("Erase Page size is %d\n", info.page_size);

    for(i=0; i < info.size; i+=info.page_size){
        if( (i % 100) == 0 ){
            printf("Erase at %ld\n", i);
        }
        memset(buffer, 0xff, info.page_size);
        eeprom.write(i, buffer, info.page_size);
        memset(buffer, 0, info.page_size);
        eeprom.read(i, buffer, info.page_size);
        for(j=0; j < info.page_size; j++){
            if( buffer[j] != 0xff ){
                printf("Failed to erase EEPROM at %ld == 0x%X\n", i+j, buffer[j]);
                exit(1);
            }
        }
    }
}

