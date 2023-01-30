#include <stdio.h>
#include "gpiod.h"
//#include <stdlib.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
////#include <time.h>

#define LEDANDBUTTONCOUNT 4

#ifndef	CONSUMER
#define	CONSUMER	"Consumer"  //what does consumer field even do?
#endif

#define SECONDSTOMICROSECONDS *1000000
#define MILISECONDS2NANOSECONDS SECONDSTOMICROSECONDS

#define LBINDEXGETRANDOM (rand()%LEDANDBUTTONCOUNT) //gets random LED/BUTTON index

int initializeLedLines(struct gpiod_chip* ptrChip,struct gpiod_line** lineLights,const int* ledsGpioNumbers);
int initializeButtonLines(struct gpiod_chip* ptrChip,struct gpiod_line** lineButtons,const int* buttonsGpioNumbers);
void releaseInitializedLedsAndButtons(struct gpiod_line** lineLights,struct gpiod_line** lineButtons);
int readDebounced(struct gpiod_line** lineButtons,int buttonIndex);

int main(void)
{
    srand(time(NULL));
    
    ///Setting up some variables:
    int returnValue = EXIT_SUCCESS;
    char* chipname="/dev/gpiochip0";
    
    const int ledsGpioNumbers[LEDANDBUTTONCOUNT]={27,22,23,24};                 //ordered by x where label is Dx
    struct gpiod_line* lineLights[LEDANDBUTTONCOUNT];    //ordered by x where label is Dx
    //the following NULLs are needed because we're checking for these when releasing resources:
    for (size_t i = 0; i < LEDANDBUTTONCOUNT; i++) { lineLights[i] = NULL; }
    //Initially, instead of the loop, I wanted to use "memset" to set all the bits to 0,
    //but, decided against it since an all-bits-0 representation of NULL is not STANDARDIZED and so
    //it is not 100% portable (well, it is extremely unlikely it isn't, but I'm taking no risks 
    //here for something so simple especially since count of LEDs and Buttons is low)
    //According to teacher: could've just used memset to set all bits to 0 because all-bits-0 can be mostly considered as a standard except for some extremely RARE cases
    //Compiler optimizes the above non-changing loop anyway

    const int buttonsGpioNumbers[LEDANDBUTTONCOUNT]={18,17,10,25};              //ordered by x where label is SWx
    struct gpiod_line* lineButtons[LEDANDBUTTONCOUNT]; //ordered by x where label is SWx
    //the following NULLs are needed because we're checking for these when releasing resources:
    for (size_t i = 0; i < LEDANDBUTTONCOUNT; i++) { lineButtons[i] = NULL; }
    //Initially, instead of the loop, I wanted to use "memset" to set all the bits to 0,
    //but, decided against it since an all-bits-0 representation of NULL is not STANDARDIZED and so
    //it is not 100% portable (well, it is extremely unlikely it isn't, but I'm taking no risks 
    //here for something so simple especially since count of LEDs and Buttons is low)
    //According to teacher: could've just used memset to set all bits to 0 because all-bits-0 can be mostly considered as a standard except for some extremely RARE cases
    // memset(lineButtons,0,LEDANDBUTTONCOUNT*sizeof(struct gpiod_line*));
    //Compiler optimizes the above non-changing loop anyway

    ///Opening the gpio chip
    struct gpiod_chip *chip;
    chip=gpiod_chip_open(chipname);
	if (!chip) 
    {
        fprintf(stderr,"Error (for %s): ",chipname);
		perror("Open chip failed");
        returnValue=EXIT_FAILURE;
		goto end;
	}



    ///Opening the lines
    //lights:
    if(initializeLedLines(chip,lineLights,ledsGpioNumbers)<0)
    {
        returnValue=EXIT_FAILURE;
        goto release_lines;
    }
    
    //buttons:
    if(initializeButtonLines(chip,lineButtons,buttonsGpioNumbers)<0)
    {
        returnValue=EXIT_FAILURE;
        goto release_lines;
    }

    
    ///Main logic:   
    ///
    struct timespec roundTimeout; roundTimeout.tv_sec=3; roundTimeout.tv_nsec=0;//You have 4 seconds per round.

    float score=0; //Correct button +1, else -0.5. //POSTPONED\DEPRECATED: wrong button -1, correct button +1, not pressing any +0.

    fprintf(stdout, "===== GAME START! =====\n INITIAL SCORE = %f\n",score);

    for(int i=0;i<20;i++) //only get 
    {
        __useconds_t microseconds2sleep= 1 SECONDSTOMICROSECONDS;
        while((microseconds2sleep=usleep(microseconds2sleep))); //usleep returns remaining time. //when microseconds2sleep ==0, we quit.
 
        int index= LBINDEXGETRANDOM; //currently supports only one led at a time, can extend to include multiple leds as time goes on for pacing sake.
        
        int retVal;
        #ifdef DEBUG
        printf("chosen index:line;%d:%d\n",index,ledsGpioNumbers[index]);
        #endif
        retVal = gpiod_line_set_value(lineLights[index], 1); //turns on led //for blinking we'd have variable val (0 or 1) and after some time we would do val = !val to change from off to on or on to off.
		if (retVal < 0) 
        {
            fprintf(stderr, "Set line (D%d) output failed (iteration i val=%d)",index+1,i);
			perror("");
            returnValue=EXIT_FAILURE;
			goto release_lines;
		}

        //input handling STARTS>>> //Alternative for the POSTPONED\DEPRECATED part: int gpiod_line_get_value_bulk(struct gpiod_line_bulk *bulk, int *values) GPIOD_API; AND WAIT FOR EVENT BULK VARIANT!

        //NOTE MIGHT NEED TO GET RID OF THIS BECAUSE OF VALUE DEBOUNCING..
        struct gpiod_line_event event;
        // while(0==gpiod_line_event_read(lineButtons[index],&event));
        retVal=gpiod_line_event_wait(lineButtons[index], &roundTimeout); //might need to pop/clear pre-existing events by reading the events..
        gpiod_line_event_read(lineButtons[index],&event);
        if(retVal==1)
        {
            int val;
            val = readDebounced(lineButtons,index);
            if (val < 0) 
            {
                fprintf(stderr, "At line %d:: readDebounced for line (SW%d) failed (iteration i val=%d)",__LINE__,index+1,i);
                perror("");
                returnValue=EXIT_FAILURE;
                goto release_lines;
            }
            if(val==0) //our button hardware has low state when pressed
            {
                score++;
                printf("HIT! New score:%f\n",score);
            }
            else //our hardware has high value when NOTpressed
            {
                score-=0.5;//should only occur when val==0;
                printf("MISSED! New score: %f\n",score);
            }
        }
        else if(retVal==0) {score-=0.1;} //missed the mole!
        else if(retVal<0){perror("Failed waiting for event");returnValue=EXIT_FAILURE;goto release_lines;}
        else{fprintf(stderr, "THIS SHOULD NEVER HAPPEN! LINE:%d\n",__LINE__);perror("ERROR!");}
        
        //input handling ENDS<<<

        retVal = gpiod_line_set_value(lineLights[index], 0); //turns off led 
        if (retVal < 0) 
        {
			perror("Set line output failed\n");
            returnValue=EXIT_FAILURE;
			goto release_lines;
		}
    }
    //some useful links:
    //https://www.ics.com/blog/gpio-programming-exploring-libgpiod-library
    //https://hackaday.com/2010/11/09/debounce-code-one-post-to-rule-them-all/
    //http://abyz.me.uk/lg/py_lgpio.html#gpio_set_debounce_micros
    ///



    /// Release lines and close chip
release_lines:
    releaseInitializedLedsAndButtons(lineLights, lineButtons);

close_chip:
    gpiod_chip_close(chip);

    

    /// Closing time...
    if(returnValue==EXIT_SUCCESS)
    {
        fprintf(stdout,"===== FINAL SCORE: %f! =====\n",score);
        fprintf(stdout,"Thanks for joining Pi 4(for) a nice game of Whackamole.\n");
    }
end:
    return returnValue;
}





int initializeLedLines(struct gpiod_chip* ptrChip,struct gpiod_line** lineLights,const int* ledsGpioNumbers)
{
    for(int i=0; i< LEDANDBUTTONCOUNT ;i++)
    {
        lineLights[i]=gpiod_chip_get_line(ptrChip,ledsGpioNumbers[i]); //learn unions in C to also refer to this by led color name?
        if (!lineLights[i]) //will be null on failure.
        {
            fprintf(stderr, "Get line for LED D%d failed.", i+1);
            perror("\n error");
            return -1;
        }
    }

    for(int i=0;i<LEDANDBUTTONCOUNT;i++)
    {
        int returned = gpiod_line_request_output(lineLights[i], CONSUMER, 0); //1);
	    if (returned < 0) 
        {
            fprintf(stderr, "Request line as output failed (D%d).", i+1);
            perror("\n error");
            return -1;
	    }
    }
    return 0;
}





int initializeButtonLines(struct gpiod_chip* ptrChip,struct gpiod_line** lineButtons,const int* buttonsGpioNumbers)
{
    for(int i=0;i<LEDANDBUTTONCOUNT;i++)
    {
        lineButtons[i]=gpiod_chip_get_line(ptrChip, buttonsGpioNumbers[i]);
        if (!lineButtons[i])
        {
            fprintf(stderr, "Get line for Button SW%d failed.\n", i+1);
            perror("error");
            return -1;
        }
    }

    for(int i=0;i<LEDANDBUTTONCOUNT;i++)
    {
        int retVal=gpiod_line_request_both_edges_events(lineButtons[i], CONSUMER);
        if(retVal<0)
        {
            fprintf(stderr, "Request both edge failed (SW%d)",i+1);
            perror("");
            return -1;
        }
    }
    // struct timespec timeoutTS;
    // timeoutTS.tv_nsec=0; timeoutTS.tv_sec=1;
    // for(int i=0; i< LEDANDBUTTONCOUNT;i++)
    // {
    //     int retVal=gpiod_line_event_wait(lineButtons[i],&timeoutTS);
    //     if(retVal<0)
    //     {
    //         perror("inital event wait");
    //         return -1;
    //     }
    // }
    return 0;
}





void releaseInitializedLedsAndButtons(struct gpiod_line** lineLights,struct gpiod_line** lineButtons)
{
    for(int i=0;i<LEDANDBUTTONCOUNT;i++)
    {
        if(lineLights[i]!=NULL)
        {
            gpiod_line_release(lineLights[i]);
        }
    }

    for(int i=0;i<LEDANDBUTTONCOUNT;i++)
    {
        if(lineButtons[i]!=NULL)
        {
            gpiod_line_release(lineButtons[i]);
        }
    }
}





int readDebounced(struct gpiod_line** lineButtons,int buttonIndex) //made with the following as its basis: https://docs.arduino.cc/built-in-examples/digital/Debounce
{
    struct timespec debounceTimeout; debounceTimeout.tv_sec=0; debounceTimeout.tv_nsec=50 MILISECONDS2NANOSECONDS;//You have 4 seconds per round.
    int tmpRetVal;
    while(true)
    {
        struct gpiod_line_event event;
        tmpRetVal=gpiod_line_event_wait(lineButtons[buttonIndex], &debounceTimeout); //Could change to bulk to get the debounce working with bulk.. Should be easy.
        if (tmpRetVal<0)
        {
            perror("gpiod line event wait (in readDebounced)");
            return -1;
        }
        else if(tmpRetVal==0) //timeout reached, signal didn't change for 50 miliseconds.
        {
            gpiod_line_event_read(lineButtons[buttonIndex],&event);
            break;
        }
        debounceTimeout.tv_nsec=50 MILISECONDS2NANOSECONDS;
    }

    gpiod_line_release(lineButtons[buttonIndex]);
    tmpRetVal = gpiod_line_request_input(lineButtons[buttonIndex], CONSUMER);
    if(tmpRetVal<0)
    {
        perror("gpiod line request input (in readDebounced)");
        lineButtons[buttonIndex]=NULL;
        return -1;
    }

    int value = gpiod_line_get_value(lineButtons[buttonIndex]); //can change to bulk variant if needed
    if (value < 0) 
    {
        fprintf(stderr, "At line %d:: Read line (SW%d) input failed",__LINE__,buttonIndex+1);
        perror("");
        return -1;
    }

    gpiod_line_release(lineButtons[buttonIndex]);
    tmpRetVal=gpiod_line_request_both_edges_events(lineButtons[buttonIndex], CONSUMER);
    if(tmpRetVal<0)
    {
        perror("gpiod line request both edges events (in readDebounced)");
        lineButtons[buttonIndex]=NULL;
        return -1;
    }

    return value;
}

