//=============================================================================
// Copyright (c) 2001-2023 FLIR Systems, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of FLIR
// Integrated Imaging Solutions, Inc. ("Confidential Information"). You
// shall not disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with FLIR Integrated Imaging Solutions, Inc. (FLIR).
//
// FLIR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. FLIR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================

/**
 *  @example Acquisition_C.c
 *
 *  @brief Acquisition_C.c shows how to acquire images. It relies on
 *  information provided in the Enumeration_C example. Following this, check
 *  out the NodeMapInfo_C example if you haven't already. It explores
 *  retrieving information from various node types.
 *
 *  This example touches on the preparation and cleanup of a camera just
 *  before and just after the acquisition of images. Image retrieval and
 *  conversion, grabbing image data, and saving images are all covered.
 *
 *  Once comfortable with Acquisition_C and NodeMapInfo_C, we suggest checking
 *  out AcquisitionMultipleCamera_C, NodeMapCallback_C, or SaveToAvi_C.
 *  AcquisitionMultipleCamera_C demonstrates simultaneously acquiring images
 *  from a number of cameras, NodeMapCallback_C acts as a good introduction to
 *  programming with callbacks and events, and SaveToAvi_C exhibits video
 *  creation.
 *
 *  Please leave us feedback at: https://www.surveymonkey.com/r/TDYMVAPI
 *  More source code examples at: https://github.com/Teledyne-MV/Spinnaker-Examples
 *  Need help? Check out our forum at: https://teledynevisionsolutions.zendesk.com/hc/en-us/community/topics
 */

#include "SpinnakerC.h"
#include "stdio.h"
#include "string.h"

// This macro helps with C-strings.
#define MAX_BUFF_LEN 256

// Use the following enum to select the stream mode
typedef enum
{
    STREAM_MODE_TELEDYNE_GIGE_VISION, // Teledyne Gige Vision stream mode is the default stream mode for spinview which is supported on Windows
    STREAM_MODE_PGRLWF, // Light Weight Filter driver is our legacy driver which is supported on Windows
    STREAM_MODE_SOCKET, // Socket is supported for MacOS and Linux, and uses native OS network sockets instead of a
                        // filter driver
} StreamMode;

#if defined(WIN32) || defined(WIN64)
const StreamMode chosenStreamMode = STREAM_MODE_TELEDYNE_GIGE_VISION;
#else
const StreamMode chosenStreamMode = STREAM_MODE_SOCKET;
#endif

// This function helps to check if a node is readable
bool8_t IsReadable(spinNodeHandle hNode, char nodeName[])
{
    spinError err = SPINNAKER_ERR_SUCCESS;
    char lastErrorMessage[MAX_BUFF_LEN];
    size_t lenLastErrorMessage = MAX_BUFF_LEN;

    bool8_t pbReadable = False;
    err = spinNodeIsReadable(hNode, &pbReadable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
    }
    return pbReadable;
}

// This function helps to check if a node is writable
bool8_t IsWritable(spinNodeHandle hNode, char nodeName[])
{
    spinError err = SPINNAKER_ERR_SUCCESS;
    bool8_t pbWritable = False;
    err = spinNodeIsWritable(hNode, &pbWritable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node writability (%s node), with error %d...\n\n", nodeName, err);
    }
    return pbWritable;
}

// This function handles the error prints when a node or entry is unavailable or
// not readable/writable on the connected camera
void PrintRetrieveNodeFailure(char node[], char name[])
{
    printf("Unable to get %s (%s %s retrieval failed).\n\n", node, name, node);
}

// Disables or enables heartbeat on GEV cameras so debugging does not incur timeout errors
spinError ConfigureGVCPHeartbeat(spinCamera hCam, bool8_t enable)
{
    spinNodeHandle hHeartbeatDisable = NULL;
    spinError err = SPINNAKER_ERR_SUCCESS;
    spinNodeHandle hDeviceType;
    char deviceType[MAX_BUFF_LEN];
    size_t lenDeviceType = MAX_BUFF_LEN;
    char lastErrorMessage[MAX_BUFF_LEN];
    size_t lenLastErrorMessage = MAX_BUFF_LEN;

    //
    // Write to boolean node controlling the camera's heartbeat
    //
    // *** NOTES ***
    // This applies only to GEV cameras.
    //
    // GEV cameras have a heartbeat built in, but when debugging applications the
    // camera may time out due to its heartbeat. Disabling the heartbeat prevents
    // this timeout from occurring, enabling us to continue with any necessary
    // debugging.
    //
    // *** LATER ***
    // Make sure that the heartbeat is reset upon completion of the debugging.
    // If the application is terminated unexpectedly, the camera may not locked
    // to Spinnaker indefinitely due to the the timeout being disabled.  When that
    // happens, a camera power cycle will reset the heartbeat to its default setting.
    //

    // Retrieve GenICam device nodemap
    spinNodeMapHandle hNodeMap = NULL;
    err = spinCameraGetNodeMap(hCam, &hNodeMap);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve GenICam nodemap. Aborting with error %d...\n\n", err);
        return err;
    }

    // Retrieve TL device nodemap
    spinNodeMapHandle hNodeMapTLDevice = NULL;
    err = spinCameraGetTLDeviceNodeMap(hCam, &hNodeMapTLDevice);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve TL device nodemap. Non-fatal error %d...\n\n", err);
        return err;
    }

    err = spinNodeMapGetNode(hNodeMapTLDevice, "DeviceType", &hDeviceType);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    err = spinNodeToString(hDeviceType, deviceType, &lenDeviceType);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    if (strcmp(deviceType, "GigEVision") != 0)
    {
        return 0;
    }

    if (enable)
    {
        printf("Resetting heartbeat...\n\n");
    }
    else
    {
        printf("Disabling heartbeat...\n\n");
    }

    err = spinNodeMapGetNode(hNodeMap, "GevGVCPHeartbeatDisable", &hHeartbeatDisable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf(
            "Unable to disable heartbeat on camera, error %d occurred. Continuing with execution as this "
            "may be non-fatal...\n\n",
            err);
        return SPINNAKER_ERR_SUCCESS;
    }

    if (!IsWritable(hHeartbeatDisable, "GevGVCPHeartbeatDisable"))
    {
        printf(
            "Unable to disable heartbeat on camera, error %d occurred. Continuing with execution as this "
            "may be non-fatal...\n\n",
            err);
        return SPINNAKER_ERR_SUCCESS;
    }

    err = spinBooleanSetValue(hHeartbeatDisable, enable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf(
            "Unable to disable heartbeat on camera (boolean node), error %d occured. Continuing with "
            "execution as this may be non-fatal...\n\n",
            err);
        return err;
    }

    if (!enable)
    {
        printf("WARNING: Heartbeat has been disabled for the rest of this example run.\n");
        printf("         Heartbeat will be reset upon the completion of this run.  If the \n");
        printf("         example is aborted unexpectedly before the heartbeat is reset, the\n");
        printf("         camera may need to be power cycled to reset the heartbeat.\n\n");
    }
    else
    {
        printf("Heartbeat has been reset.\n\n");
    }

    return err;
}

spinError ResetGVCPHeartbeat(spinCamera hCam)
{
    return ConfigureGVCPHeartbeat(hCam, True);
}

spinError DisableGVCPHeartbeat(spinCamera hCam)
{
    return ConfigureGVCPHeartbeat(hCam, False);
}

spinError SetStreamMode(spinCamera hCam)
{
    // *** NOTES ***
    // Enumeration nodes are slightly more complicated to set than other
    // nodes. This is because setting an enumeration node requires working
    // with two nodes instead of the usual one.
    //
    // As such, there are a number of steps to setting an enumeration node:
    // retrieve the enumeration node from the nodemap, retrieve the desired
    // entry node from the enumeration node, retrieve the integer value
    // from the entry node, and set the new value of the enumeration node
    // with the integer value from the entry node.
    //
    // It is important to note that there are two sets of functions that might
    // produce erroneous results if they were to be mixed up. The first two
    // functions, spinEnumerationSetIntValue() and
    // spinEnumerationEntryGetIntValue(), use the integer values stored on each
    // individual cameras. The second two, spinEnumerationSetEnumValue() and
    // spinEnumerationEntryGetEnumValue(), use enum values defined in the
    // Spinnaker library. The int and enum values will most likely be
    // different from another.
    //

    spinError err = SPINNAKER_ERR_SUCCESS;
    spinNodeHandle hStreamMode = NULL;
    spinNodeHandle hStreamModeCustom = NULL;
    int64_t streamModeCustom = 0;
    char lastErrorMessage[MAX_BUFF_LEN];
    size_t lenLastErrorMessage = MAX_BUFF_LEN;
    char currentEntrySymbolic[MAX_BUFF_LEN];
    size_t currentEntrySymbolicLength = MAX_BUFF_LEN;
    size_t displayNameLength = MAX_BUFF_LEN;
    spinNodeHandle hCurrentEntryNode = NULL;
    spinNodeMapHandle hNodeMapStreamDevice = NULL;

    err = spinCameraGetTLStreamNodeMap(hCam, &hNodeMapStreamDevice);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    // The node "StreamMode" is only available for GEV cameras.
    err = spinNodeMapGetNode(hNodeMapStreamDevice, "StreamMode", &hStreamMode);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return 0;
    }

    // Skip setting stream mode if the node is inaccessible.
    if (hStreamMode == NULL || !IsReadable(hStreamMode, "StreamMode") || (!IsWritable(hStreamMode, "StreamMode")))
    {
        return 0;
    }

    const char* streamMode;
    switch (chosenStreamMode)
    {
    case STREAM_MODE_PGRLWF:
        streamMode = "LWF";
        break;
    case STREAM_MODE_SOCKET:
        streamMode = "Socket";
        break;
    case STREAM_MODE_TELEDYNE_GIGE_VISION:
    default:
        streamMode = "TeledyneGigeVision";
    }

    // Retrieve the desired entry node
    err = spinEnumerationGetEntryByName(hStreamMode, streamMode, &hStreamModeCustom);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    // Retrieve the desired entry node value from the entry node
    err = spinEnumerationEntryGetIntValue(hStreamModeCustom, &streamModeCustom);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    // Set integer as new value for enumeration node
    err = spinEnumerationSetIntValue(hStreamMode, streamModeCustom);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    // Print our result
    err = spinEnumerationGetCurrentEntry(hStreamMode, &hCurrentEntryNode);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    err = spinEnumerationEntryGetSymbolic(hCurrentEntryNode, currentEntrySymbolic, &currentEntrySymbolicLength);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    printf("Stream Mode set to %s\n", currentEntrySymbolic);

    return err;
}

// This function acquires and saves 10 images from a device.
spinError AcquireImages(spinCamera hCam, spinNodeMapHandle hNodeMap, spinNodeMapHandle hNodeMapTLDevice)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    printf("\n*** IMAGE ACQUISITION ***\n\n");

    //
    // Set acquisition mode to continuous
    //
    // *** NOTES ***
    // Because the example acquires and saves 10 images, setting acquisition
    // mode to continuous lets the example finish. If set to single frame
    // or multiframe (at a lower number of images), the example would just
    // hang. This would happen because the example has been written to acquire
    // 10 images while the camera would have been programmed to retrieve
    // less than that.
    //
    // Setting the value of an enumeration node is slightly more complicated
    // than other node types, and especially so in C. It can roughly be broken
    // down into four steps: first, the enumeration node is retrieved from the
    // nodemap; second, the entry node is retrieved from the enumeration node;
    // third, an integer is retrieved from the entry node; and finally, the
    // integer is set as the new value of the enumeration node.
    //
    // It is important to note that there are two sets of functions that might
    // produce erroneous results if they were to be mixed up. The first two
    // functions, spinEnumerationSetIntValue() and
    // spinEnumerationEntryGetIntValue(), use the integer values stored on each
    // individual cameras. The second two, spinEnumerationSetEnumValue() and
    // spinEnumerationEntryGetEnumValue(), use enum values defined in the
    // Spinnaker library. The int and enum values will most likely be
    // different from another.
    //
    spinNodeHandle hAcquisitionMode = NULL;
    spinNodeHandle hAcquisitionModeContinuous = NULL;
    int64_t acquisitionModeContinuous = 0;
    char lastErrorMessage[MAX_BUFF_LEN];
    size_t lenLastErrorMessage = MAX_BUFF_LEN;

    // Retrieve enumeration node from nodemap
    err = spinNodeMapGetNode(hNodeMap, "AcquisitionMode", &hAcquisitionMode);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    // Retrieve entry node from enumeration node
    if (IsReadable(hAcquisitionMode, "AcquisitionMode"))
    {
        err = spinEnumerationGetEntryByName(hAcquisitionMode, "Continuous", &hAcquisitionModeContinuous);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
            printf("Error: %s [%d]\n\n", lastErrorMessage, err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "AcquisitionMode");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // Retrieve integer from entry node
    if (IsReadable(hAcquisitionModeContinuous, "AcquisitionModeContinuous"))
    {
        err = spinEnumerationEntryGetIntValue(hAcquisitionModeContinuous, &acquisitionModeContinuous);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
            printf("Error: %s [%d]\n\n", lastErrorMessage, err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "AcquisitionMode 'Continuous'");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }
    // Set integer as new value of enumeration node
    if (IsWritable(hAcquisitionMode, "AcquisitionMode"))
    {
        err = spinEnumerationSetIntValue(hAcquisitionMode, acquisitionModeContinuous);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
            printf("Error: %s [%d]\n\n", lastErrorMessage, err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "AcquisitionMode");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    printf("Acquisition mode set to continuous...\n");

    //
    // Begin acquiring images
    //
    // *** NOTES ***
    // What happens when the camera begins acquiring images depends on the
    // acquisition mode. Single frame captures only a single image, multi
    // frame catures a set number of images, and continuous captures a
    // continuous stream of images. Because the example calls for the retrieval
    // of 10 images, continuous mode has been set.
    //
    // *** LATER ***
    // Image acquisition must be ended when no more images are needed.
    //
    err = spinCameraBeginAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    printf("Acquiring images...\n");

    //
    // Retrieve device serial number for filename
    //
    // *** NOTES ***
    // The device serial number is retrieved in order to keep cameras from
    // overwriting one another. Grabbing image IDs could also accomplish this.
    //
    spinNodeHandle hDeviceSerialNumber = NULL;
    char deviceSerialNumber[MAX_BUFF_LEN];
    size_t lenDeviceSerialNumber = MAX_BUFF_LEN;

    err = spinNodeMapGetNode(hNodeMapTLDevice, "DeviceSerialNumber", &hDeviceSerialNumber);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        strcpy(deviceSerialNumber, "");
        lenDeviceSerialNumber = 0;
    }
    else
    {
        if (IsReadable(hDeviceSerialNumber, "DeviceSerialNumber"))
        {
            err = spinStringGetValue(hDeviceSerialNumber, deviceSerialNumber, &lenDeviceSerialNumber);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                strcpy(deviceSerialNumber, "");
                lenDeviceSerialNumber = 0;
            }
        }
        else
        {
            strcpy(deviceSerialNumber, "");
            lenDeviceSerialNumber = 0;
            PrintRetrieveNodeFailure("node", "DeviceSerialNumber");
        }
        printf("Device serial number retrieved as %s...\n", deviceSerialNumber);
    }
    printf("\n");

    // Retrieve, convert, and save images
    const unsigned int k_numImages = 10;
    unsigned int imageCnt = 0;

    //
    // Create Image Processor context for post processing images
    //
    spinImageProcessor hImageProcessor = NULL;
    err = spinImageProcessorCreate(&hImageProcessor);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to create image processor. Non-fatal error %d...\n\n", err);
    }

    //
    // Set default image processor color processing method
    //
    // *** NOTES ***
    // By default, if no specific color processing algorithm is set, the image
    // processor will default to NEAREST_NEIGHBOR method.
    //
    err = spinImageProcessorSetColorProcessing(hImageProcessor, SPINNAKER_COLOR_PROCESSING_ALGORITHM_HQ_LINEAR);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to set image processor color processing method. Non-fatal error %d...\n\n", err);
    }

    for (imageCnt = 0; imageCnt < k_numImages; imageCnt++)
    {
        //
        // Retrieve next received image
        //
        // *** NOTES ***
        // Capturing an image houses images on the camera buffer. Trying to
        // capture an image that does not exist will hang the camera.
        //
        // *** LATER ***
        // Once an image from the buffer is saved and/or no longer needed, the
        // image must be released in orer to keep the buffer from filling up.
        //
        spinImage hResultImage = NULL;

        err = spinCameraGetNextImageEx(hCam, 1000, &hResultImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
            printf("Error: %s [%d]\n\n", lastErrorMessage, err);
            continue;
        }

        //
        // Ensure image completion
        //
        // *** NOTES ***
        // Images can easily be checked for completion. This should be done
        // whenever a complete image is expected or required. Further, check
        // image status for a little more insight into why an image is
        // incomplete.
        //
        bool8_t isIncomplete = False;
        bool8_t hasFailed = False;

        err = spinImageIsIncomplete(hResultImage, &isIncomplete);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
            printf("Error: %s [%d]\n\n", lastErrorMessage, err);
            hasFailed = True;
        }

        // Check image for completion
        if (isIncomplete)
        {
            spinImageStatus imageStatus = SPINNAKER_IMAGE_STATUS_NO_ERROR;

            err = spinImageGetStatus(hResultImage, &imageStatus);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("Unable to retrieve image status. Non-fatal error %d...\n\n", imageStatus);
            }
            else
            {
                printf("Image incomplete with image status %d...\n", imageStatus);
            }

            hasFailed = True;
        }

        // Release incomplete or failed image
        if (hasFailed)
        {
            err = spinImageRelease(hResultImage);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
                printf("Error: %s [%d]\n\n", lastErrorMessage, err);
            }

            continue;
        }

        //
        // Print image information; height and width recorded in pixels
        //
        // *** NOTES ***
        // Images have quite a bit of available metadata including things such
        // as CRC, image status, and offset values, to name a few.
        //
        size_t width = 0;
        size_t height = 0;

        printf("Grabbed image %d, ", imageCnt);

        // Retrieve image width
        err = spinImageGetWidth(hResultImage, &width);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("width = unknown, ");
        }
        else
        {
            printf("width = %u, ", (unsigned int)width);
        }

        // Retrieve image height
        err = spinImageGetHeight(hResultImage, &height);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("height = unknown\n");
        }
        else
        {
            printf("height = %u\n", (unsigned int)height);
        }

        //
        // Convert image to mono 8
        //
        // *** NOTES ***
        // Images not gotten from a camera directly must be created and
        // destroyed. This includes any image copies, conversions, or
        // otherwise. Basically, if the image was gotten, it should be
        // released, if it was created, it needs to be destroyed.
        //
        // Images can be converted between pixel formats by using the
        // appropriate enumeration value. Unlike the original image, the
        // converted one does not need to be released as it does not affect the
        // camera buffer.
        //
        // Optionally, the color processing algorithm can also be set using
        // the alternate spinImageConvertEx() function.
        //
        // *** LATER ***
        // The converted image was created, so it must be destroyed to avoid
        // memory leaks.
        //
        spinImage hConvertedImage = NULL;

        err = spinImageCreateEmpty(&hConvertedImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
            printf("Error: %s [%d]\n\n", lastErrorMessage, err);
            hasFailed = True;
        }

        err = spinImageProcessorConvert(hImageProcessor, hResultImage, hConvertedImage, PixelFormat_Mono8);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
            printf("Error: %s [%d]\n\n", lastErrorMessage, err);
            hasFailed = True;
        }

        // Create a unique filename
        char filename[MAX_BUFF_LEN];

        if (lenDeviceSerialNumber == 0)
        {
            sprintf(filename, "Acquisition-C-%d.jpg", imageCnt);
        }
        else
        {
            sprintf(filename, "Acquisition-C-%s-%d.jpg", deviceSerialNumber, imageCnt);
        }

        //
        // Save image
        //
        // *** NOTES ***
        // The standard practice of the examples is to use device serial
        // numbers to keep images of one device from overwriting those of
        // another.
        //
        err = spinImageSave(hConvertedImage, filename, SPINNAKER_IMAGE_FILE_FORMAT_JPEG);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
            printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        }
        else
        {
            printf("Image saved at %s\n\n", filename);
        }

        //
        // Destroy converted image
        //
        // *** NOTES ***
        // Images that are created must be destroyed in order to avoid memory
        // leaks.
        //
        err = spinImageDestroy(hConvertedImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
            printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        }

        //
        // Release image from camera
        //
        // *** NOTES ***
        // Images retrieved directly from the camera (i.e. non-converted
        // images) need to be released in order to keep from filling the
        // buffer.
        //
        err = spinImageRelease(hResultImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
            printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        }
    }

    //
    // Destroy Image Processor context
    //
    // *** NOTES ***
    // Image processor context needs to be destroyed after all image processing
    // are complete to avoid memory leaks.
    //
    err = spinImageProcessorDestroy(hImageProcessor);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to destroy image processor. Non-fatal error %d...\n\n", err);
    }

    //
    // End acquisition
    //
    // *** NOTES ***
    // Ending acquisition appropriately helps ensure that devices clean up
    // properly and do not need to be power-cycled to maintain integrity.
    //
    err = spinCameraEndAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
    }

    return err;
}

// This function prints the device information of the camera from the transport
// layer; please see NodeMapInfo_C example for more in-depth comments on
// printing device information from the nodemap.
spinError PrintDeviceInfo(spinNodeMapHandle hNodeMap)
{
    spinError err = SPINNAKER_ERR_SUCCESS;
    unsigned int i = 0;
    char lastErrorMessage[MAX_BUFF_LEN];
    size_t lenLastErrorMessage = MAX_BUFF_LEN;

    printf("\n*** DEVICE INFORMATION ***\n\n");

    // Retrieve device information category node
    spinNodeHandle hDeviceInformation = NULL;

    err = spinNodeMapGetNode(hNodeMap, "DeviceInformation", &hDeviceInformation);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    // Retrieve number of nodes within device information node
    size_t numFeatures = 0;

    if (IsReadable(hDeviceInformation, "DeviceInformation"))
    {
        err = spinCategoryGetNumFeatures(hDeviceInformation, &numFeatures);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
            printf("Error: %s [%d]\n\n", lastErrorMessage, err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("node", "DeviceInformation");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // Iterate through nodes and print information
    for (i = 0; i < numFeatures; i++)
    {
        spinNodeHandle hFeatureNode = NULL;

        err = spinCategoryGetFeatureByIndex(hDeviceInformation, i, &hFeatureNode);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
            printf("Error: %s [%d]\n\n", lastErrorMessage, err);
            continue;
        }

        spinNodeType featureType = UnknownNode;

        // get feature node name
        char featureName[MAX_BUFF_LEN];
        size_t lenFeatureName = MAX_BUFF_LEN;
        err = spinNodeGetName(hFeatureNode, featureName, &lenFeatureName);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            strcpy(featureName, "Unknown name");
        }

        if (IsReadable(hFeatureNode, featureName))
        {
            err = spinNodeGetType(hFeatureNode, &featureType);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
                printf("Error: %s [%d]\n\n", lastErrorMessage, err);
                continue;
            }
        }
        else
        {
            printf("%s: Node not readable\n", featureName);
            continue;
        }

        char featureValue[MAX_BUFF_LEN];
        size_t lenFeatureValue = MAX_BUFF_LEN;

        err = spinNodeToString(hFeatureNode, featureValue, &lenFeatureValue);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            strcpy(featureValue, "Unknown value");
        }

        printf("%s: %s\n", featureName, featureValue);
    }
    printf("\n");

    return err;
}

// This function acts as the body of the example; please see NodeMapInfo_C
// example for more in-depth comments on setting up cameras.
spinError RunSingleCamera(spinCamera hCam)
{
    spinError err = SPINNAKER_ERR_SUCCESS;
    // Retrieve TL device nodemap and print device information
    spinNodeMapHandle hNodeMapTLDevice = NULL;
    char lastErrorMessage[MAX_BUFF_LEN];
    size_t lenLastErrorMessage = MAX_BUFF_LEN;

    err = spinCameraGetTLDeviceNodeMap(hCam, &hNodeMapTLDevice);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
    }
    else
    {
        err = PrintDeviceInfo(hNodeMapTLDevice);
    }

    // Initialize camera
    err = spinCameraInit(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    // Retrieve GenICam nodemap
    spinNodeMapHandle hNodeMap = NULL;

    err = spinCameraGetNodeMap(hCam, &hNodeMap);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    // Configure heartbeat for GEV camera
#ifdef _DEBUG
    err = DisableGVCPHeartbeat(hCam);
#else
    err = ResetGVCPHeartbeat(hCam);
#endif
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return err;
    }

    // Set stream mode
    err = SetStreamMode(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return err;
    }

    // Acquire images
    err = AcquireImages(hCam, hNodeMap, hNodeMapTLDevice);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return err;
    }

#ifdef _DEBUG
    // Reset heartbeat for GEV camera
    err = ResetGVCPHeartbeat(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return err;
    }
#endif

    // Deinitialize camera
    err = spinCameraDeInit(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    return err;
}

// Example entry point; please see Enumeration_C example for more in-depth
// comments on preparing and cleaning up the system.
int main(/*int argc, char** argv*/)
{
    spinError errReturn = SPINNAKER_ERR_SUCCESS;
    spinError err = SPINNAKER_ERR_SUCCESS;
    unsigned int i = 0;
    char lastErrorMessage[MAX_BUFF_LEN];
    size_t lenLastErrorMessage = MAX_BUFF_LEN;

    // Since this application saves images in the current folder
    // we must ensure that we have permission to write to this folder.
    // If we do not have permission, fail right away.
    FILE* tempFile;
    tempFile = fopen("test.txt", "w+");
    if (tempFile == NULL)
    {
        printf("Failed to create file in current folder.  Please check "
               "permissions.\n");
        printf("Press Enter to exit...\n");
        getchar();
        return SPINNAKER_ERR_ACCESS_DENIED;
    }
    fclose(tempFile);
    remove("test.txt");

    // Print application build information
    printf("Application build date: %s %s \n\n", __DATE__, __TIME__);

    // Retrieve singleton reference to system object
    spinSystem hSystem = NULL;

    err = spinSystemGetInstance(&hSystem);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    // Print out current library version
    spinLibraryVersion hLibraryVersion;

    spinSystemGetLibraryVersion(hSystem, &hLibraryVersion);
    printf(
        "Spinnaker library version: %d.%d.%d.%d\n\n",
        hLibraryVersion.major,
        hLibraryVersion.minor,
        hLibraryVersion.type,
        hLibraryVersion.build);

    // Retrieve list of cameras from the system
    spinCameraList hCameraList = NULL;

    err = spinCameraListCreateEmpty(&hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    err = spinSystemGetCameras(hSystem, hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    // Retrieve number of cameras
    size_t numCameras = 0;

    err = spinCameraListGetSize(hCameraList, &numCameras);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    printf("Number of cameras detected: %u\n\n", (unsigned int)numCameras);

    // Finish if there are no cameras
    if (numCameras == 0)
    {
        // Clear and destroy camera list before releasing system
        err = spinCameraListClear(hCameraList);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
            printf("Error: %s [%d]\n\n", lastErrorMessage, err);
            return err;
        }

        err = spinCameraListDestroy(hCameraList);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
            printf("Error: %s [%d]\n\n", lastErrorMessage, err);
            return err;
        }

        // Release system
        err = spinSystemReleaseInstance(hSystem);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
            printf("Error: %s [%d]\n\n", lastErrorMessage, err);
            return err;
        }

        printf("Not enough cameras!\n");
        printf("Done! Press Enter to exit...\n");
        getchar();

        return -1;
    }

    // Run example on each camera
    for (i = 0; i < numCameras; i++)
    {
        printf("\nRunning example for camera %d...\n", i);

        // Select camera
        spinCamera hCamera = NULL;

        err = spinCameraListGet(hCameraList, i, &hCamera);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
            printf("Error: %s [%d]\n\n", lastErrorMessage, err);
            errReturn = err;
        }
        else
        {
            // Run example
            err = RunSingleCamera(hCamera);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                errReturn = err;
            }
        }

        // Release camera
        err = spinCameraRelease(hCamera);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = err;
        }

        printf("Camera %d example complete...\n\n", i);
    }

    // Clear and destroy camera list before releasing system
    err = spinCameraListClear(hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    err = spinCameraListDestroy(hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    // Release system
    err = spinSystemReleaseInstance(hSystem);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        spinErrorGetLastMessage(lastErrorMessage, &lenLastErrorMessage);
        printf("Error: %s [%d]\n\n", lastErrorMessage, err);
        return err;
    }

    printf("\nDone! Press Enter to exit...\n");
    getchar();

    return errReturn;
}
