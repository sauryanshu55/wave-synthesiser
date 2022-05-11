# wave-synthesiser
A C terminal program that creates a .wav file. Takes in arguments as waveforms: sine, square, triangle and output filename. &amp; saw. <br>
Included is a makefile in the terminal that can be called with the command
```
$> make
```
Makefile has the command 
```
$> gcc synth.c -o synth -lm -fsanitize=address
```
This creates a file called ./synth, which can then be used in the form:
```
$> ./synth -w <FORM: either sine, square, triangle or saw> -f <frequency> -d <duration> -p <pulse> <Output filename.wav>
 ```
For example:
```
$> ./synth -w sine -f 330 -d 5 -p 1 sample-sine-wav.wav 
```
_Make sure to include the .wav extension in the outputfile name!_<br>
The <b>-fsanitize=address</b> flag aborts the program if pre-conditions, i.e valid arguments in proper flags, are not provided. Minor memory leaks may occur in that case.<br>
Else, memory is not leaked, and output file is proportional to data inside the .wav file.

<b>Terminal screenshots provided below:</b>
<br>
![image](https://user-images.githubusercontent.com/95371800/167761504-c38e0abf-6d05-4135-85f6-c25b04d5676b.png)

