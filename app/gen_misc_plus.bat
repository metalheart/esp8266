rm ..\bin\upgrade\%1.bin

cd .output\eagle\debug\image\

xt-objcopy -j .text eagle.app.v6.out eagle.app.v6.text.out
xt-objcopy -S -g -O binary eagle.app.v6.text.out eagle.app.v6.text.bin

xt-objcopy -j .data eagle.app.v6.out eagle.app.v6.data.out
xt-objcopy -S -g -O binary eagle.app.v6.data.out eagle.app.v6.data.bin

xt-objcopy -j .rodata eagle.app.v6.out eagle.app.v6.rodata.out
xt-objcopy -S -g -O binary eagle.app.v6.rodata.out eagle.app.v6.rodata.bin

xt-objcopy -j .irom0.text eagle.app.v6.out eagle.app.v6.irom0text.out
xt-objcopy -S -g -O binary eagle.app.v6.irom0text.out eagle.app.v6.irom0text.bin

..\..\..\..\..\tools\gen_appbin.py eagle.app.v6.out v6

..\..\..\..\..\tools\gen_flashbin.py eagle.app.v6.flash.bin eagle.app.v6.irom0text.bin

cp eagle.app.flash.bin %1.bin

xcopy /y %1.bin ..\..\..\..\..\bin\upgrade\

cd ..\..\..\..\
