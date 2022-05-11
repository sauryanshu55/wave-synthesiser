#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

//Returns true of the argument of the form flag is valid
bool is_valid_form_arg(char *formArg);


//header ADT, alias header
typedef struct header{
    char *MARKER; //marker, 4 bytes, 32 bits-> ALWAYS "RIFF", no '\0'
    uint32_t CHUNK_SZ; //chunk size, 4 bytes, 32 bits -> size of the total file + header
    unsigned char *FRMT; //format, 4 bytes, 32 bits-> ALWAYS "WAVE", no '\0'
    
    //METADATA
    unsigned char *SUBCHUNK_ID; //first subchunk id, 4 bytes, 32 bits->ALWAYS "fmt ", no '\0'
    uint32_t SUBCHUNK_ID_SZ; //subchunk id size, 4 bytes, 32 bits->Size of first subchunk in bytes, ALWAYS 16 bytes
    uint16_t AUDIO_FRMT; //audio format, 2 bytes, 16 bits-> Type of audio format, ALWAYS 1
    uint16_t NUM_CHNLS; //number of chanels, 2 bytes, 16 bits-> ALWAYS 1
    uint32_t SAMPLE_RT; //sample rate, 4 bytes, 32 bits->ALWAYS 44100
    uint32_t BYTE_RT; //byte rate, 4 bytes, 32 bits-> sample rate * number of channels *bytes per sample
    uint16_t BLK_ALGMNT; //block alignment, 2 bytes, 16 bits -> numbers of channels *bytes per sample 
    uint16_t BITS_PER_SMPL; //bits per sample, 2 bytes, 16 bits-> ALWAYS 32

    //SECOND SUBCHUNK
    unsigned char *SCND_SUBCHUNK_ID; //second subchunk id, 4 bytes, 32 bits -> ALWAYS "DATA" w/o '\0'
    uint32_t SCND_SUBCHUNK_SZ; //second subchunk size, 4 bytes, 32 bits ->size of the payload, other than the header
} header;

//Writes to the header file, returns a pointer to a header file

//initializes the headerf file
header init_header(uint32_t file_size);

// Returns the number of samples contained a wave file (with a sample rate of
// 44.1 kHz and 1 channel) that plays for dur seconds.
int32_t num_samples_from_duration(int dur);

// Writes a wave file header to `file` appropriate for a sound data payload of
// size sz.
void write_header(uint32_t file_size,FILE *wavFile);

// Returns a pointer to a heap-allocated buffer containing the sound data for
// a simple triangle wave that lasts for dur seconds.
uint32_t* make_simple_sawtooth_data(int dur);

// Writes a complete wave file to a file named filename using the sound data
// pointed to by data of which contains len samples.
void write_wav(const char *output_filename, int32_t len, uint32_t *data);

// Given a float f in the range [-1.0, 1.0], return a sample in the range
// [0, INT32_MAX].
uint32_t convert_to_sample(float f);

// Returns a pointer to a heap-allocated buffer containing the sound data for
// a sine wave of frequency freq that lasts for dur seconds.
uint32_t* make_sine_data(int dur, int freq);

// Returns a pointer to a heap-allocated buffer containing the sound data for
// a saw wave of frequency freq that lasts for dur seconds.
uint32_t* make_saw_data(int dur, int freq);


// Returns a pointer to a heap-allocated buffer containing the sound data for
// a square wave of frequency freq that lasts for dur seconds.
uint32_t* make_square_data(int dur, int freq);

//Returns INT32_MAX-1 if float>=0, else, returns 0
uint32_t convert_to_square_sample(float val);

// Returns a pointer to a heap-allocated buffer containing the sound data for
// a triangle wave of frequency freq that lasts for dur seconds.
uint32_t* make_triangle_data(int dur, int freq);

// Returns a pointer to a heap-allocated buffer containing the sound data for
// a saw wave of frequency freq that lasts for dur seconds.
uint32_t* make_saw_data(int dur, int freq);

//Flips the booleans of the bools listed as params, used to change gradient inn make_triangle_data
void flip_derivatives(bool increasing_derivative, bool decreasing_derivative);

// Modifies the sound data found in data of length len to have n pulses of sound
// using a simple linearly decaying envelope.
void apply_pulse_envelope(uint32_t *raw_data, int len, int n);

//Adds post processing UI comments
void post_processing(char* formArg, int freqArg, int durArg, int pulseArg, char* output_filename);
// ./ssynth -f 300 -w sine -d 3 -p 5