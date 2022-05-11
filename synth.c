#include "synth.h"
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//COMMENTS INCLUDED IN HEADER FILE!


header init_header(uint32_t file_size) {
    header retHeader;
    retHeader.MARKER = "RIFF";
    retHeader.CHUNK_SZ = file_size;
    retHeader.FRMT = "WAVE";

    // Metadata
    retHeader.SUBCHUNK_ID = "fmt ";
    retHeader.SUBCHUNK_ID_SZ = 16;
    retHeader.AUDIO_FRMT = 1;
    retHeader.NUM_CHNLS = 1;
    retHeader.SAMPLE_RT = 44100;

    int byte_rate = retHeader.SAMPLE_RT * retHeader.NUM_CHNLS * sizeof(uint32_t) * 8;
    retHeader.BYTE_RT = byte_rate;

    int block_alignment = retHeader.NUM_CHNLS * sizeof(uint32_t) * 8;
    retHeader.BLK_ALGMNT = block_alignment;
    retHeader.BITS_PER_SMPL = 32;

    retHeader.SCND_SUBCHUNK_ID = "data";
    retHeader.SCND_SUBCHUNK_SZ = file_size - 44;

    return retHeader;
}

int32_t num_samples_from_duration(int dur) {
    return 44100 * 1 * dur;  // SMPL_RT * NUM_CHNLS *dur
}

uint32_t convert_to_sample(float val) {
    val = val + 1;                // Adding one to the vallue to remove the negative sign
    val = val * (INT32_MAX - 1);  // amplifying float val by (INT32_MAX-1) so that the effect of casting to uint32_t, which will remove the decimals,will be negligible
    uint32_t ret_converted = (uint32_t)val;
    return ret_converted;
}

uint32_t *make_sine_data(int dur, int freq) {
    int numSamples = num_samples_from_duration(dur);
    float const PI = 3.14159265359;
    double curPhase = 0;
    double freq_doubleType = (float)freq;
    double changePhase = 2 * PI * (freq_doubleType / 44100);

    uint32_t *ret_sine_data = (uint32_t *)malloc(sizeof(uint32_t) * numSamples);  //[5] malloc, will be borrowed

    for (int i = 1; i <= numSamples; i++) {
        if (curPhase >= (2 * PI)) {
            curPhase = curPhase - (2 * PI);  // if curPhase >> than the allowed phase, it subtracts 2 PI
        }
        float sine_val = sin(curPhase);                         // sine is a function of the phase currently on, curPhase
        uint32_t data_to_assign = convert_to_sample(sine_val);  // converting to sample,adds 1 and multiplies the amp by INT32_MAX -1
        *ret_sine_data = data_to_assign;
        // printf("Sample data [%d]= %u\n",i,*ret_sine_data);
        ret_sine_data++;
        curPhase = curPhase + changePhase;  // changing the curphase to thr next curphase by adding the appropriate change in phase
    }

    for (int i = 1; i <= numSamples; i++) {
        ret_sine_data--;  // Returning pointer to the start of the heap
    }
    return ret_sine_data;
}

uint32_t convert_to_square_sample(float val) {
    uint32_t ret_converted;
    if (val >= 0) {
        ret_converted = INT32_MAX - 1;
        // printf("Before casting: %u\n",ret_converted );
    } else {
        ret_converted = 0;
        // printf("Before casting: %u\n",ret_converted );
    }
    // printf("After casting: %u\n",ret_converted );
    return ret_converted;
}
uint32_t *make_square_data(int dur, int freq) {
    int numSamples = num_samples_from_duration(dur);
    float const PI = 3.14159265359;
    double curPhase = 0;
    double freq_doubleType = (float)freq;
    double changePhase = 2 * PI * (freq_doubleType / 44100);

    uint32_t *ret_square_data = (uint32_t *)malloc(sizeof(uint32_t) * numSamples);  //[3] heap allocated malloc, will be borrowed

    for (int i = 1; i <= numSamples; i++) {
        float sine_val = sin(curPhase);
        uint32_t data_to_assign = convert_to_square_sample(sine_val);
        *ret_square_data = data_to_assign;
        // printf("Sample data [%d]= %u\n",i,*ret_square_data);
        ret_square_data++;
        curPhase = curPhase + changePhase;  // changing the curphase to thr next curphase by adding the appropriate change in phase
    }

    for (int i = 1; i <= numSamples; i++) {
        ret_square_data--;  // Returning pointer to the start of the heap
    }
    return ret_square_data;
}

void flip_derivatives(bool increasing_derivative, bool decreasing_derivative) {
    if (increasing_derivative == true && decreasing_derivative == false) {
        increasing_derivative = false;
        decreasing_derivative = true;
    }
    if (increasing_derivative == false && decreasing_derivative == true) {
        increasing_derivative = true;
        decreasing_derivative = false;
    }
}

uint32_t *make_triangle_data(int dur, int freq) {
    int numSamples = num_samples_from_duration(dur);
    float const PI = 3.14159265359;

    float num_samples_per_frequency = numSamples / freq;  // number of samples of frequency, so thhat we can find out the quarters of the frequency where the gradients flip
    // printf("Num samples per freq: %f\n",num_samples_per_frequency);
    float period = num_samples_per_frequency / 4;  // This is the period in which the gradients must change/flip
    // printf("period: %f\n",period);
    uint32_t additionConst = (INT32_MAX - 1) / (period);  // This constant will be added in each iteration
    // printf("addition constant: %u\n",additionConst);

    int nthSample = 0;  // nth sample will be used to count where in the phase we are so that once it exceeds the period, we can flip/change
    uint32_t data_to_assign = 0;
    uint32_t *ret_triangle_data = (uint32_t *)malloc(sizeof(uint32_t) * numSamples);  //[4] malloc, will be borrowed

    bool increasing_derivative = true;
    bool decreasing_derivative = false;

    for (int i = 1; i <= numSamples; i++) {
        if (increasing_derivative == true && decreasing_derivative == false) {
            data_to_assign = data_to_assign + additionConst;
        }
        if (increasing_derivative == false && increasing_derivative == true) {
            data_to_assign = data_to_assign - additionConst;
        }
        *ret_triangle_data = data_to_assign;
        ret_triangle_data++;
        nthSample++;

        if (nthSample > period) {
            flip_derivatives(increasing_derivative, decreasing_derivative);
            nthSample = 0;
        }
    }

    for (int i = 1; i <= numSamples; i++) {
        ret_triangle_data--;
    }
    return ret_triangle_data;
}

uint32_t *make_saw_data(int dur, int freq) {
    int numSamples = num_samples_from_duration(dur);
    float const PI = 3.14159265359;

    float num_samples_per_frequency = numSamples / freq;
    float period = num_samples_per_frequency / 4;
    uint32_t additionConst = (INT32_MAX - 1) / (period);

    int nthSample = 0;  // nth sample will be used to count where in the phase we are so that once it exceeds the period, we can flip/change
    uint32_t data_to_assign = 0;
    uint32_t *ret_triangle_data = (uint32_t *)malloc(sizeof(uint32_t) * numSamples);  //[2] malloc, will be borrowed

    bool increasing_derivative = true;
    bool decreasing_derivative = false;

    for (int i = 1; i <= numSamples; i++) {
        data_to_assign = data_to_assign + additionConst;
        *ret_triangle_data = data_to_assign;
        // printf("sample[%d]: %u\n",i,*ret_triangle_data);
        ret_triangle_data++;
        nthSample++;

        if (nthSample > period) {
            nthSample = 0;
            data_to_assign = 0;
        }
    }

    for (int i = 1; i <= numSamples; i++) {
        ret_triangle_data--;
    }
    return ret_triangle_data;
}

uint32_t *make_simple_sawtooth_data(int dur) {
    int numSamples = num_samples_from_duration(dur);
    uint32_t *ret_sawtooth_data = (uint32_t *)malloc(sizeof(uint32_t) * numSamples);  // [1] malloc, will be borrowed

    uint32_t data_to_assign = 0;

    for (int i = 0; i < numSamples; i++) {
        *ret_sawtooth_data = data_to_assign;
        data_to_assign = data_to_assign + 8000000;
        ret_sawtooth_data++;
    }

    for (int i = 0; i < numSamples; i++) {
        // printf("%u\n",*ret_sawtooth_data);
        ret_sawtooth_data--;  // returning ret to the original point!
    }
    return ret_sawtooth_data;
}

float *generate_enveloping_data(int len, int pulseArg) {
    // Audio track has len samples. If pulseArg=1, then numrical distance between each discrete values is 1/len
    // If audio track has len samples and pulseArg>1, then track is seperated to pulseArg divisions, each division going from 1 to 0 multiplier values.
    // So each discrete value of envelope multipliers is seperated by: 1/(len/pulseArg)=pulseArg/len, which is equal to the downward gradient

    float pulseArg_as_float = (float)pulseArg;
    float len_as_float = (float)len;
    float enveloping_gradient = pulseArg_as_float / len_as_float;

    int numEnvelopes = 0;                                             // Keeps track of the number of envelopes created so far, to macth to len samples
    float data_to_assign = 1;                                         // First pulse envelope
    float *ret_envelope_data = (float *)malloc(sizeof(float) * len);  //[6] malloc, to be borrowed

    while (numEnvelopes != len)  // Ensures that all sample values have an enveloping multiplier
    {
        *ret_envelope_data = data_to_assign;
        numEnvelopes++;
        ret_envelope_data++;
        data_to_assign -= enveloping_gradient;  // decreasing the assigned amp value by gradient

        if (data_to_assign <= 0) {  // Ideally data_to_assign would stop at zero, but for sake of minimizing errors when its <=0, it resets back to one. The number of these resets should ideally be = pulseArg
            data_to_assign = 1;
        }
    }

    while (numEnvelopes != 0) {
        ret_envelope_data--;  // returning to original pointer
        numEnvelopes--;
    }
    return ret_envelope_data;
}

void apply_pulse_envelope(uint32_t *raw_data, int len, int pulseArg) {
    float *pulse_envelope = generate_enveloping_data(len, pulseArg);  //[7] malloc, won't be borrowed

    for (int i = 1; i <= len; i++) {
        (*raw_data) = (*raw_data) * (*pulse_envelope);  // multiplies the raw data with the pulsed values
        raw_data++;
        pulse_envelope++;
    }

    for (int i = 1; i <= len; i++) {
        raw_data--;
        pulse_envelope--;
    }
    free(pulse_envelope);
}

void write_wav(const char *output_filename, int32_t len, uint32_t *data) {
    FILE *wavFile;
    wavFile = fopen(output_filename, "wb");
    write_header(sizeof(uint32_t) * len, wavFile);
    fwrite(data, sizeof(uint32_t), len, wavFile);
    fclose(wavFile);
}

void write_header(uint32_t file_size, FILE *wavFile) {
    header header_attr = init_header(file_size);

    fwrite(header_attr.MARKER, 1, 4, wavFile);
    fwrite(&header_attr.CHUNK_SZ, sizeof(header_attr.CHUNK_SZ), 1, wavFile);
    fwrite(header_attr.FRMT, 1, 4, wavFile);

    fwrite(header_attr.SUBCHUNK_ID, 1, 4, wavFile);
    fwrite(&header_attr.SUBCHUNK_ID_SZ, sizeof(header_attr.SUBCHUNK_ID_SZ), 1, wavFile);
    fwrite(&header_attr.AUDIO_FRMT, sizeof(header_attr.AUDIO_FRMT), 1, wavFile);
    fwrite(&header_attr.NUM_CHNLS, sizeof(header_attr.NUM_CHNLS), 1, wavFile);
    fwrite(&header_attr.SAMPLE_RT, sizeof(header_attr.SAMPLE_RT), 1, wavFile);
    fwrite(&header_attr.BYTE_RT, sizeof(header_attr.BYTE_RT), 1, wavFile);
    fwrite(&header_attr.BLK_ALGMNT, sizeof(header_attr.BLK_ALGMNT), 1, wavFile);
    fwrite(&header_attr.BITS_PER_SMPL, sizeof(header_attr.BITS_PER_SMPL), 1, wavFile);

    fwrite(header_attr.SCND_SUBCHUNK_ID, 1, 4, wavFile);
    fwrite(&header_attr.SCND_SUBCHUNK_SZ, sizeof(header_attr.SCND_SUBCHUNK_SZ), 1, wavFile);
}

// printf("calc value is : %lf\n",phaseChange);
bool is_valid_form_arg(char *formArg) {
    if ((strcasecmp("sine", formArg) == 0) || (strcasecmp("triangle", formArg) == 0) || (strcasecmp("saw", formArg) == 0) || (strcasecmp("square", formArg) == 0)) {
        return true;
    }
    return false;
}

void post_processing(char* formArg, int freqArg, int durArg, int pulseArg, char* output_filename){
    printf("Usage: ./synth -w <Wave Form> -f <frequency> -d <duration> -p <pulse> <FILENAME>\n");
    printf("Arguments passed:\n");
    printf("-f <form>           \033[0;31m%s Type\033\033[0;37m\n",formArg);
    printf("-f <freq>           \033[0;31m%d Hz\033\033[0;37m\n",freqArg);
    printf("-w <dur>            \033[0;31m%ds\033\033[0;37m\n",durArg);
    printf("-w <pulse>          \033[0;31m%d Times\033\033[0;37m\n",pulseArg);
    printf("Output file will be named as: \033[0;31m%s\033\033[0;37m\n", output_filename);
}

bool check_file_creation(char *output_filename){
    FILE *wavFile;
    wavFile=fopen(output_filename,"rb");
    if (wavFile==NULL){
        printf("File Creation failed! \n");\
        return false;
    }
    else{
        printf("File creation successful!\n");
    }
}

void main(int argc, char *argv[]) {
    // Bool variables set to false, true unless args set
    bool is_form_set = false;
    bool is_freq_set = false;
    bool is_dur_set = false;
    bool is_pulse_set = false;

    // setting default values, will change if args present
    char *formArg = "sine";
    float freqArg = 440.0;
    int durArg = 3;
    int pulseArg = 1;
    char *output_filename;

    int opt;

    while ((opt = getopt(argc, argv, "w:f:d:p:")) != -1) {
        switch (opt) {
            case 'w':
                if (is_valid_form_arg(optarg)) {
                    formArg = optarg;
                    is_form_set = true;
                } else {
                    is_form_set = true;
                    formArg = "sine";
                }
                break;

            case 'f':
                if (optarg == NULL) {
                    is_freq_set = true;
                    freqArg = 440.0;
                } else if (atof(optarg) == 0) {
                    is_freq_set = true;
                } else {
                    is_freq_set = true;
                    freqArg = atof(optarg);
                }
                break;

            case 'd':
                if (optarg == NULL) {
                    is_dur_set = true;
                    freqArg = 3;
                } else if (atoi(optarg) == 0) {
                    is_dur_set = true;
                    durArg = 3;
                } else {
                    is_dur_set = true;
                    durArg = atoi(optarg);
                }
                break;

            case 'p':
                if (optarg == NULL) {
                    is_pulse_set = true;
                    pulseArg = 1;
                } else if (atoi(optarg) == 0) {
                    is_pulse_set = true;
                    pulseArg = 1;
                } else {
                    is_pulse_set = true;
                    pulseArg = atoi(optarg);
                }
                break;
        }
    }

    if (optind < argc) {
        output_filename = argv[9];
    } else {
        output_filename = "synth.wav";
    }
    post_processing(formArg,freqArg,durArg,pulseArg,output_filename);
    int len = num_samples_from_duration(durArg);
    if (strcasecmp(formArg, "sine") == 0) {
        // FOR SINE DATA
        uint32_t *sine_data = make_sine_data(durArg, freqArg);
        apply_pulse_envelope(sine_data,len,pulseArg);
        write_wav(output_filename, len, sine_data);  //[5] malloc, sine_data is owner
        free(sine_data);                             //[5] heap malloc freed
    } else if (strcasecmp(formArg, "saw")==0) {
        // FOR SAW DATA:
        uint32_t *saw_data = make_saw_data(durArg, freqArg);  //[2] heap allocated malloc is now owned by saw_data
        apply_pulse_envelope(saw_data,len,pulseArg);
        write_wav(output_filename, len, saw_data);
        free(saw_data);  //[2] heap malloc freed

    } else if (strcasecmp(formArg, "triangle")==0) {
        // FOR TRIANGLE DATA
        uint32_t *triangle_data = make_triangle_data(durArg, freqArg);
        apply_pulse_envelope(triangle_data,len,pulseArg);
        write_wav(output_filename, len, triangle_data);  //[4] heap allocated malloc is now owned by triangle_data
        free(triangle_data);  //[4] heap malloc freed
    }

    else if (strcasecmp(formArg, "square")==0) {
        // FOR SQUARE DATA
        uint32_t *square_data = make_square_data(durArg, freqArg);  //[3] heap allocated malloc is now owned by square_data
        apply_pulse_envelope(square_data,len,pulseArg);
        write_wav(output_filename, len, square_data);
        free(square_data);
    }
    check_file_creation(output_filename);
    printf("\n");
}
