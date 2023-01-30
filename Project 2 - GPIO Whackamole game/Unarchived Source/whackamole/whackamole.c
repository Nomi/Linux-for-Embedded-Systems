//NOTE: A proper debouncing would use the wait bulk for each line and keep track of which buttons are stable and for each button, trigger the action it needs to trigger when it is stable.

//Debouncing in this version doesn't work properly in all cases. Therefore, I'll fix it some other time. Maybe with a different library than libgpiod or even create my own layer on top of it (could've solved everything using multithreading as well (one thread per button and one main thread to keep track of those)).

#include <stdio.h>
#include "gpiod.h"
//#include <stdlib.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
////#include <time.h>

/**
 * Number of LEDs and Buttons
 */
#define LBCOUNT 4 //Leds and Buttons count.

#ifndef	CONSUMER
#define	CONSUMER	"Consumer"  //what does consumer field even do?
#endif

#define SECONDSTOMICROSECONDS *1000000
#define MILISECONDS2NANOSECONDS SECONDSTOMICROSECONDS

/**
 *@returns: Returns a random index within the range: [0,LBCOUNT]
 */
#define LBINDEXGETRANDOM (rand()%LBCOUNT) //gets random LED/BUTTON index


//Can't we just use "uint"?
#define UINT unsigned int
// typedef unsigned int UINT;
typedef struct timespec timespec_t;


////Structures:

// //will make other structures for clean code after implementing multi-line reading.

// typedef struct _lights
// {
//     UINT led_number;
//     struct gpiod_line *line;
// } lights;

// typedef struct _button
// {
//     UINT button_number;
//     struct gpiod_line *line;
//     struct gpiod_line_event event;
// } button;

// const lights LEDS[LBCOUNT] = {
//     {22},
//     {24}};

// const button BUTTONS[LBCOUNT] = {
//     {25},
//     {10}};


// typedef struct buttonCallBackArgs
// {
//     struct gpiod_line_bulk *bulk;
//     struct gpiod_chip *chip;
//     button **buttons;
// } _btnCallBack;


////Function declarations:
int initializeLedLines(struct gpiod_chip* ptrChip,struct gpiod_line** lineLights,const int* ledsGpioNumbers);
// int initializeLedLines(struct gpiod_chip* ptrChip, light* lineLights);
int initializeButtonLines(struct gpiod_chip* ptrChip,struct gpiod_line** lineButtons,const int* buttonsGpioNumbers);
void releaseInitializedLedsAndButtons(struct gpiod_line** lineLights,struct gpiod_line** lineButtons);
int readDebounced(struct gpiod_line** lineButtons,int buttonIndex);


////Main function:
int main(void)
{
    srand(time(NULL));
    ///Setting up some variables:
    int returnValue = EXIT_SUCCESS;
    char* chipname="/dev/gpiochip0";
    
    const int ledsGpioNumbers[LBCOUNT]={27,22,23,24};                 //ordered by x where label is Dx
    struct gpiod_line* lineLights[LBCOUNT];    //ordered by x where label is Dx
    //the following NULLs are needed because we're checking for these when releasing resources:
    for (size_t i = 0; i < LBCOUNT; i++) { lineLights[i] = NULL; }
    //Initially, instead of the loop, I wanted to use "memset" to set all the bits to 0,
    //but, decided against it since an all-bits-0 representation of NULL is not STANDARDIZED and so
    //it is not 100% portable (well, it is extremely unlikely it isn't, but I'm taking no risks 
    //here for something so simple especially since count of LEDs and Buttons is low)
    //According to teacher: could've just used memset to set all bits to 0 because all-bits-0 can be mostly considered as a standard except for some extremely RARE cases
    //Compiler optimizes the above non-changing loop anyway

    const int buttonsGpioNumbers[LBCOUNT]={18,17,10,25};              //ordered by x where label is SWx
    struct gpiod_line* lineButtons[LBCOUNT]; //ordered by x where label is SWx
    //the following NULLs are needed because we're checking for these when releasing resources:
    for (size_t i = 0; i < LBCOUNT; i++) { lineButtons[i] = NULL; }
    //Same comments for this as the similar forloop a few lines above this one..

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

    ///STUFF FOR BULK: //what about release?
    struct gpiod_line_bulk bulk = GPIOD_LINE_BULK_INITIALIZER;
    gpiod_line_bulk_init(&bulk);
    struct gpiod_line_bulk event_bulk = GPIOD_LINE_BULK_INITIALIZER;
    gpiod_line_bulk_init(&event_bulk);
    struct gpiod_line_event events[16];

    for(int i=0;i<LBCOUNT;i++)
    {
        gpiod_line_bulk_add(&bulk, lineButtons[i]);
    }
    ///Main logic:   
    ///
    struct timespec roundTimeout;

    float score=0; //Correct button +3, else -0.5. //POSTPONED\DEPRECATED: wrong button -1, correct button +1, not pressing any +0.

    fprintf(stdout, "===== GAME START! =====\n INITIAL SCORE = %f\n",score);


    for(int i=0;i<7;i++) //only get 
    {
        roundTimeout.tv_sec=5; roundTimeout.tv_nsec=0;//You have 5 seconds per round.

        event_bulk.num_lines = 0;
        memset(event_bulk.lines, 0, GPIOD_LINE_BULK_MAX_LINES * sizeof(struct gpiod_line *));
        __useconds_t microseconds2sleep= 3 SECONDSTOMICROSECONDS;
        while((microseconds2sleep=usleep(microseconds2sleep))); //usleep returns remaining time. //when microseconds2sleep ==0, we quit.
        // fprintf(stderr,"memsetted");
        int selectedIndex= LBINDEXGETRANDOM; //currently supports only one led at a time, can extend to include multiple leds as time goes on for pacing sake.
        
        int retVal;
        #ifdef DEBUG
        printf("chosen index:line;%d:%d\n",index,ledsGpioNumbers[index]);
        #endif
        retVal = gpiod_line_set_value(lineLights[selectedIndex], 1); //turns on led //for blinking we'd have variable val (0 or 1) and after some time we would do val = !val to change from off to on or on to off.
		if (retVal < 0) 
        {
            fprintf(stderr, "Set line (D%d) output failed (iteration i val=%d)",selectedIndex+1,i);
			perror("");
            returnValue=EXIT_FAILURE;
			goto release_lines;
		}

        //input handling STARTS>>> //Alternative for the POSTPONED\DEPRECATED part: int gpiod_line_get_value_bulk(struct gpiod_line_bulk *bulk, int *values) GPIOD_API; AND WAIT FOR EVENT BULK VARIANT!

        //NOTE MIGHT NEED TO GET RID OF THIS BECAUSE OF VALUE DEBOUNCING..
        //struct gpiod_line_event event;
        //// while(0==gpiod_line_event_read(lineButtons[index],&event));
        // retVal=gpiod_line_event_wait(lineButtons[index], &roundTimeout); //might need to pop/clear pre-existing events by reading the events..
        // gpiod_line_event_read(lineButtons[index],&event);
        retVal = gpiod_line_event_wait_bulk(&bulk, &roundTimeout, &event_bulk);
        if(retVal==0) {score-=0.1; printf("MISSED! New score: %f\n",score);} //missed the mole!
        else if(retVal<0){perror("Failed waiting for event");returnValue=EXIT_FAILURE;goto release_lines;}
        else if(retVal==1)
        {
            int hitDetected=0;
            const int num_lines = gpiod_line_bulk_num_lines(&event_bulk);
            // struct gpiod_line *line = gpiod_line_bulk_get_line(&event_bulk, i);
            int val;
            // int sumOtherButtonEvents=0;
            int rv;
            // for(int i=0; i < event_bulk.num_lines;i++)
            // {
            //     struct gpiod_line *line = gpiod_line_bulk_get_line(&event_bulk, i);
            //     //******line->offset //gives number of gpio it's associated iwth
            //     int tmpret=gpiod_line_event_read_multiple(line, events, 16);
            //     if (tmpret < 0)
            //     {
            //         perror("error reading line events");
            //         returnValue=EXIT_FAILURE;
            //         goto release_lines;
            //     }
            //     sumOtherButtonEvents+=tmpret;
            // }

            struct gpiod_line_event events[16];
            struct gpiod_line *line, **lineptr;
            gpiod_line_bulk_foreach_line(&event_bulk, line, lineptr) //if there's a crash, I suspect because I release a line, open it as input, release it again, open it as events, without changing the bulk object at all
            {
                int readFallingEdge=0;
                int isCorrectLine=0;
                int currentLineOffset=gpiod_line_offset(line);
                if(currentLineOffset==buttonsGpioNumbers[selectedIndex])
                    isCorrectLine=1;
                rv = gpiod_line_event_read_multiple(line, events, 16);
                if (rv < 0)
                {
                    perror("error reading line events");
                    returnValue=EXIT_FAILURE;
                    goto release_lines;
                }
                // else if(rv==0)                 //the case rv==0 shouldn't be hit in our function.
                // {
                //     score-=1;
                //     printf("WRONG button pressed! New score: %f\n",score);
                // }
                else if(rv>0)
                {
                    // fprintf(stderr, 'checking events->event_type\n');
                    for (int i = 0; i < rv; i++)
                    {
                        if(events[i].event_type==2)
                            readFallingEdge=1;
                    }

                    struct gpiod_line* singleLineArray[1];
                    singleLineArray[0] = line;
                    val = readDebounced(singleLineArray,0);
                    if(isCorrectLine) //My program simultane
                    {
                        if(readFallingEdge) //correct button was PRESSED DOWN
                        {
                            // int rv=99; //==CORRECT (obviously)(Just a reminder)
                            if (val < 0) 
                            {
                                fprintf(stderr, "At line %d:: readDebounced for line (SW%d) failed (iteration i val=%d)",__LINE__,selectedIndex+1,i);
                                perror("");
                                returnValue=EXIT_FAILURE;
                                goto release_lines;
                            }
                            else if(val==0)//DEPRECATED: the following comment is deprecated because I changed how readDebounced works (it doesn't read anymore)//our button hardware has low state when pressed
                            {
                                hitDetected=1;
                                score+=3;
                                printf("HIT! New score:%f\n",score);
                            }
                            //the following else should not be needed as the program is now. I'll keep it here uncommented in case I might have to change the program during the lab and it ends up being reliant on the following.
                            
                            //actually, it will occur if the user stops pressing the button before debouncing finishes (in ~50ms)
                        }
                        else
                        {
                            if (val < 0) 
                            {
                                fprintf(stderr, "At line %d:: readDebounced failed (gpiod_line offset=%d,iteration i val=%d)",__LINE__,currentLineOffset,i);
                                perror("");
                                returnValue=EXIT_FAILURE;
                                goto release_lines;
                            }
                            printf("HEY, NO CHEATING! The button was already pressed before the round or have an inhumane reaction speed.\n");
                        }
                    }
                    else //not correct line
                    {
                        if (val < 0) 
                        {
                            fprintf(stderr, "At line %d:: readDebounced failed (gpiod_line offset=%d,iteration i val=%d)",__LINE__,currentLineOffset,i);
                            perror("");
                            returnValue=EXIT_FAILURE;
                            goto release_lines;
                        }
                        else if(readFallingEdge)
                        {
                            score-=1;
                            printf("WRONG button pressed! New score: %f\n",score);
                        }
                    }
                }
            }
            if(!hitDetected) //DEPRECATED COMMENT: //DEPRECATED COMMENT: should also handle the case rv==0 because hitDetected is changed to true (1) from false (0) only in some specific cases WHEN rv>0, and rv is an error which quits program. 
            {
                score-=0.5;//should only occur when val==0;
                printf("MISSED! New score: %f\n",score);
            }
        }
        else{fprintf(stderr, "THIS SHOULD NEVER HAPPEN! LINE:%d\n",__LINE__);perror("ERROR!");}
        
        //input handling ENDS<<<

        retVal = gpiod_line_set_value(lineLights[selectedIndex], 0); //turns off led 
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


//////      ======          REMEMBER TO ADD RETURNS VALUES (for errors as well) TO JSDOC COMMENTS LATER         ======      //////


////Function definitions:

//JsDoc html or markdown formatting doesn't seem to work (at least not with "clangd" and/or "vscode" (or their combination)).
/**
 *@ptrChip - Pointer to the chip we're using to access buttons.
 *@lineLights - Array in which the lines of lights are stored.
 *@ledsGpioNumbers - Array of integers representing the GPIO pin number of the LEDs (in order of the lab RaspBerryPi boards Dx naming scheme)
 *
 *DETAILS:
 *Get lines for all lights and set all of them to off.
 *
 *NOTE: 
 *depends on the value of LBCOUNT macro/definition.
 */
int initializeLedLines(struct gpiod_chip* ptrChip,struct gpiod_line** lineLights,const int* ledsGpioNumbers)
{
    for(int i=0; i< LBCOUNT ;i++)
    {
        lineLights[i]=gpiod_chip_get_line(ptrChip,ledsGpioNumbers[i]); //learn unions in C to also refer to this by led color name?
        if (!lineLights[i]) //will be null on failure.
        {
            fprintf(stderr, "Get line for LED D%d failed.", i+1);
            perror("\n error");
            return -1;
        }
    }

    for(int i=0;i<LBCOUNT;i++)
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

//JsDoc html or markdown formatting doesn't seem to work (at least not with "clangd" and/or "vscode" (or their combination)).

/**
 *@ptrChip - Pointer to the chip we're using to access buttons.
 *@lineButtons - Array in which the lines of buttons are stored.
 *@buttonsGpioNumbers -  Array of integers storing the GPIO pin number of the buttons (in order of the lab RaspBerryPi boards SW naming scheme)
 *
 *DETAILS:
 *Get lines and request events of both edges on all the buttons in the lineButtons array.
 *
 *NOTE: 
 *depends on the value of LBCOUNT macro/definition.
 */
int initializeButtonLines(struct gpiod_chip* ptrChip,struct gpiod_line** lineButtons,const int* buttonsGpioNumbers)
{
    for(int i=0;i<LBCOUNT;i++)
    {
        lineButtons[i]=gpiod_chip_get_line(ptrChip, buttonsGpioNumbers[i]);
        if (!lineButtons[i])
        {
            fprintf(stderr, "Get line for Button SW%d failed.\n", i+1);
            perror("error");
            return -1;
        }
    }

    for(int i=0;i<LBCOUNT;i++)
    {
        // // int returned=gpiod_line_request_input(lineButtons[i], CONSUMER);
        // // if (returned < 0) 
        // // {
        // //     fprintf(stderr, "Request line as input failed (SW%d)\n",i+1);
        // //     perror("error");
        // //     return -1;
        // // }
        // int retVal=gpiod_line_request_falling_edge_events(lineButtons[i], CONSUMER);
        int retVal=gpiod_line_request_both_edges_events(lineButtons[i], CONSUMER);
        if(retVal<0)
        {
            fprintf(stderr, "Request both edge failed (SW%d)",i+1); //fprintf(stderr, "Request falling edge failed (SW%d)",i+1); fprintf(stderr, "Request input failed (SW%d)",i+1);
            perror("");
            return -1;
        }
    }
    // struct timespec timeoutTS;
    // timeoutTS.tv_nsec=0; timeoutTS.tv_sec=1;
    // for(int i=0; i< LBCOUNT;i++)
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

//JsDoc html or markdown formatting doesn't seem to work (at least not with "clangd" and/or "vscode" (or their combination)).

/**
 *@param lineLights - Array of lines of lights
 *@param lineButtons - Array of lines of buttons
 *
 *DETAILS:
 *Releases all previously reserved LEDs and Button lines.
 *
 *NOTE: 
 *depends on the value of LBCOUNT macro/definition.
 *
 *@example:
 *   releaseInitializedLedsAndButtons(allLights,allButtons)
 */
void releaseInitializedLedsAndButtons(struct gpiod_line** lineLights,struct gpiod_line** lineButtons)
{
    for(int i=0;i<LBCOUNT;i++)
    {
        if(lineLights[i]!=NULL)
        {
            gpiod_line_release(lineLights[i]);
        }
    }

    for(int i=0;i<LBCOUNT;i++)
    {
        if(lineButtons[i]!=NULL)
        {
            gpiod_line_release(lineButtons[i]);
        }
    }
}



/**
 * @param lineButtons - array of lines to the buttons.
 * @param buttonIndex - index of the button to be read.
 * @brief Reads input after debouncing (uses libgpiod).
 * @return returns the state of the button when succesful, or -1 when an error occurs (and prints those errors to stderr).
*/
int readDebounced(struct gpiod_line** lineButtons,int buttonIndex) //made with the following as its basis: https://docs.arduino.cc/built-in-examples/digital/Debounce
{
    // fprintf(stderr,"read-debounced\n");
    struct timespec debounceTimeout; debounceTimeout.tv_sec=0; debounceTimeout.tv_nsec=50 MILISECONDS2NANOSECONDS;
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
            break;
        }
        else //events have occured, so debouncing not finished yet.
        {
            struct gpiod_line_event events[16];
            int retRetRet;
            while((retRetRet=gpiod_line_event_read_multiple(lineButtons[buttonIndex], events, 16)) != 0) //clearing out all the events by reading them.
            {
                //NOTE TO SELF!!:: !!!!!!!IMPRTANT!!!!!!!!check if the gpiod sets an event? using retRetRet
                memset(events,0,16*sizeof(struct gpiod_line_event));//preparing the events array for next iteration of the loop.
            }
        }
        debounceTimeout.tv_nsec=50 MILISECONDS2NANOSECONDS;
    }
    return 0;
}

/// Will do the following some other day!
// int bulkDebounce(struct gpiod_line** lineButtons,struct gpiod_line_bulk* pbulk, struct gpiod_line_bulk* pevent_bulk)
// {
//     struct timespec debounceTimeout; debounceTimeout.tv_sec=0; debounceTimeout.tv_nsec=50 MILISECONDS2NANOSECONDS;
//     int tmpRetVal;
//     time_t
//     while(true)
// }

// int readBulkDebounced(struct gpiod_line** lineButtons,int buttonIndex,  struct gpiod_line_bulk* pbulk, struct gpiod_line_bulk* pevent_bulk) //made with the following as its basis: https://docs.arduino.cc/built-in-examples/digital/Debounce
// {
//     struct timespec debounceTimeout; debounceTimeout.tv_sec=0; debounceTimeout.tv_nsec=50 MILISECONDS2NANOSECONDS;//You have 4 seconds per round.
//     int tmpRetVal;
//     while(true)
//     {
//         struct gpiod_line_event event;
//         //tmpRetVal=gpiod_line_event_wait(lineButtons[buttonIndex], &debounceTimeout); //Could change to bulk to get the debounce working with bulk.. Should be easy.
//         tmpRetVal=gpiod_line_event_wait_bulk(pbulk, &debounceTimeout, pevent_bulk);
//         if (tmpRetVal<0)
//         {
//             perror("gpiod line event wait bulk (in readBulkDebounced)");
//             return -1;
//         }
//         else if(tmpRetVal==0) //timeout reached, signal didn't change for 50 miliseconds.
//         {
//             gpiod_line_event_read(lineButtons[buttonIndex],&event);
//             break;
//         }
//         debounceTimeout.tv_nsec=50 MILISECONDS2NANOSECONDS;
//     }

//     gpiod_line_release(lineButtons[buttonIndex]);
//     tmpRetVal = gpiod_line_request_input(lineButtons[buttonIndex], CONSUMER);
//     if(tmpRetVal<0)
//     {
//         perror("gpiod line request input (in readBulkDebounced)");
//         lineButtons[buttonIndex]=NULL;
//         return -1;
//     }

//     int value = gpiod_line_get_value(lineButtons[buttonIndex]); //can change to bulk variant if needed
//     if (value < 0) 
//     {
//         fprintf(stderr, "At line %d:: Read line (SW%d) input failed",__LINE__,buttonIndex+1);
//         perror("");
//         return -1;
//     }

//     gpiod_line_release(lineButtons[buttonIndex]);
//     tmpRetVal=gpiod_line_request_both_edges_events(lineButtons[buttonIndex], CONSUMER);
//     if(tmpRetVal<0)
//     {
//         perror("gpiod line request both edges events (in readBulkDebounced)");
//         lineButtons[buttonIndex]=NULL;
//         return -1;
//     }

//     return value;
// }

