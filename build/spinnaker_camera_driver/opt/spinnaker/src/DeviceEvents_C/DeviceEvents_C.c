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
 *  @example DeviceEvents_C.c
 *
 *  @brief DeviceEvents_C.c shows how to create and use device events. It
 *  relies on information provided in the Enumeration_C, Acquisition_C, and
 *  NodeMapInfo_C examples.
 *
 *  It can also be helpful to familiarize yourself with the NodeMapCallback_C
 *  example, as nodemap callbacks follow the same general procedure as
 *  events, but with a few less steps.
 *
 *  Device events can be thought of as camera-related events. Events generally
 *  require a class to be defined as an event handler; however, because C is
 *  not an object-oriented language, an event context is created using a
 *  function and a struct whereby the function acts as the event method and
 *  the struct as its properties.
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

// This function helps to check if a node is readable
bool8_t IsReadable(spinNodeHandle hNode, char nodeName[])
{
    spinError err = SPINNAKER_ERR_SUCCESS;
    bool8_t pbReadable = False;
    err = spinNodeIsReadable(hNode, &pbReadable);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node readability (%s node), with error %d...\n\n", nodeName, err);
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

// Use the following enum and global constant to select whether the device
// event is registered universally to all events or specifically to exposure
// end events.
typedef enum _deviceEventType
{
    GENERIC,
    SPECIFIC
} deviceEventType;

const deviceEventType chosenEvent = GENERIC;

// This struct represents the properties of what would be a device event handler
// were we working in an object-oriented programming language. The struct is
// created with a pointer and passed into the function, which creates persistent
// data, mimicking properties of a class.
typedef struct _userData
{
    int count;
    char eventName[MAX_BUFF_LEN];
} userData;

// This function represents what would be the method of a device event handler.
// Together with the struct above, this makes up the device event context.
// Notice that the function signature must match this exactly for the
// function to be accepted when creating the event.
void onDeviceEvent(const spinDeviceEventData hEventData, const char* pEventName, void* pUserData)
{
    spinError err = SPINNAKER_ERR_SUCCESS;
    userData* eventInfo = (userData*)pUserData;

    if (strcmp(pEventName, eventInfo->eventName) == 0)
    {
        //
        // Retrieve event ID
        //
        // *** NOTES ***
        // Additional information can be retrieved with the device event handler
        // including event ID, payload, and payload size. These functions must
        // only be called within events.
        //
        uint64_t eventID = 0;

        err = spinDeviceEventGetId(hEventData, &eventID);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("\tCould not grab device event ID.\n\n");
        }

        // Print information on specified device event
        printf("\tDevice event %s with ID %u number %d...\n", pEventName, (unsigned int)eventID, eventInfo->count++);
    }
    else
    {
        // Print no information on non-specified information
        printf("\tDevice event occurred; not %s; ignoring...", eventInfo->eventName);
    }
}

// This function configures the example to execute device events by enabling all
// types of device events, and then creating and registering a device event handler that
// that only concerns itself with an end of exposure event.
spinError ConfigureDeviceEvents(
    spinNodeMapHandle hNodeMap,
    spinCamera hCam,
    spinDeviceEventHandler* deviceEventHandler,
    userData* eventInfo)
{
    spinError err = SPINNAKER_ERR_SUCCESS;
    unsigned int i = 0;

    printf("\n\n*** DEVICE EVENTS CONFIGURATION ***\n\n");

    //
    // Retrieve device event selector
    //
    // *** NOTES ***
    // Each type of device event must be enabled individually. This is done by
    // retrieving "EventSelector" (an enumeration node) and then enabling the
    // device event on "EventNotification" (another enumeration node).
    //
    // This example only deals with exposure end events. However, instead of
    // only enabling exposure end events with a simpler device event function,
    // all device events are enabled while the device event handler deals with
    // ensuring that only exposure end events are considered. A more standard
    // use-case might be to enable only the events of interest.
    //
    // Retrieve selector node
    spinNodeHandle hEventSelector = NULL;

    err = spinNodeMapGetNode(hNodeMap, "EventSelector", &hEventSelector);
    if (SPINNAKER_ERR_SUCCESS != err)
    {
        printf("Unable to retrieve selector. Aborting with error %d...\n\n", err);
        return err;
    }

    // Retrieve number of entries
    size_t numEntries = 0;
    if (IsReadable(hEventSelector, "EventSelector"))
    {
        err = spinEnumerationGetNumEntries(hEventSelector, &numEntries);
        if (SPINNAKER_ERR_SUCCESS != err)
        {
            printf("Unable to retrieve number of entries. Aborting with error %d...\n\n", err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("node", "EventSelector");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }
    printf("Enabling events...\n");

    //
    // Enable device events
    //
    // *** NOTES ***
    // In order to enable a device event, the event selector and event
    // notification nodes (both of type enumeration) must work in unison. The
    // desired event must first be selected on the event selector node and then
    // enabled on the event notification node.
    //
    for (i = 0; i < numEntries; i++)
    {
        // Select entry on event selector node
        spinNodeHandle hEntry = NULL;
        char entryName[MAX_BUFF_LEN];
        size_t lenEntryName = MAX_BUFF_LEN;
        int64_t value = 0;

        err = spinEnumerationGetEntryByIndex(hEventSelector, i, &hEntry);
        if (SPINNAKER_ERR_SUCCESS != err)
        {
            printf("Unable to select entry (enum entry node retrieval). Aborting with error %d...\n\n", err);
            return err;
        }

        if (IsReadable(hEntry, "EventSelector 'Entry'"))
        {
            err = spinNodeGetDisplayName(hEntry, entryName, &lenEntryName);
            if (SPINNAKER_ERR_SUCCESS != err)
            {
                printf("Unable to select entry (enum entry name retrieval). Aborting with error %d...\n\n", err);
                return err;
            }
        }
        else
        {
            PrintRetrieveNodeFailure("entry", "EntrySelector 'Entry'");
            continue;
        }

        err = spinEnumerationEntryGetIntValue(hEntry, &value);
        if (SPINNAKER_ERR_SUCCESS != err)
        {
            printf("Unable to select entry (enum entry int value retrieval). Aborting with error %d...\n\n", err);
            return err;
        }

        if (IsWritable(hEventSelector, "EventSelector"))
        {
            err = spinEnumerationSetIntValue(hEventSelector, value);
            if (SPINNAKER_ERR_SUCCESS != err)
            {
                printf("Unable to select entry (enum entry setting). Aborting with error %d...\n\n", err);
                return err;
            }
        }
        else
        {
            PrintRetrieveNodeFailure("node", "EventSelector");
            continue;
        }

        // Enable entry on event notification node
        spinNodeHandle hEventNotification = NULL;
        spinNodeHandle hEventNotificationOn = NULL;
        int64_t eventNotificationOn = 0;

        err = spinNodeMapGetNode(hNodeMap, "EventNotification", &hEventNotification);
        if (SPINNAKER_ERR_SUCCESS != err)
        {
            printf("Unable to enable entry (node retrieval). Aborting with error %d...\n\n", err);
            return err;
        }

        // retrieve event notifiaction on
        if (IsReadable(hEventNotification, "EventNotification"))
        {
            err = spinEnumerationGetEntryByName(hEventNotification, "On", &hEventNotificationOn);
            if (SPINNAKER_ERR_SUCCESS != err)
            {
                printf("Unable to enable entry (enum entry retrieval). Aborting with error %d...\n\n", err);
                return err;
            }
        }
        else
        {
            PrintRetrieveNodeFailure("node", "EventNotification");
            return SPINNAKER_ERR_ACCESS_DENIED;
        }

        if (IsReadable(hEventNotificationOn, "EventNotificationOn"))
        {
            err = spinEnumerationEntryGetIntValue(hEventNotificationOn, &eventNotificationOn);
            if (SPINNAKER_ERR_SUCCESS != err)
            {
                printf("Unable to enable entry (enum entry int value retrieval). Aborting with error %d...\n\n", err);
                return err;
            }
        }
        else
        {
            PrintRetrieveNodeFailure("entry", "EventNotification 'On'");
            return SPINNAKER_ERR_ACCESS_DENIED;
        }

        if (IsWritable(hEventNotification, "EventNotification"))
        {
            err = spinEnumerationSetIntValue(hEventNotification, eventNotificationOn);
            if (SPINNAKER_ERR_SUCCESS != err)
            {
                printf("Unable to enable entry (enum entry setting). Aborting with error %d...\n\n", err);
                return err;
            }

            printf("\t%s enabled...\n", entryName);
        }
        else
        {
            PrintRetrieveNodeFailure("node", "EventNotification");
            return SPINNAKER_ERR_ACCESS_DENIED;
        }
    }

    //
    // Prepare user data
    //
    // *** NOTES ***
    // It is important to ensure that all requisite variables are initialized
    // appropriately before creating the device event.
    //
    // *** LATER ***
    // It is a good idea to keep this data in scope in order to avoid memory
    // leaks.
    //
    eventInfo->count = 0;

    strcpy(eventInfo->eventName, "EventExposureEnd");

    //
    // Create device event handler
    //
    // *** NOTES ***
    // The device event handler function has been written to only print information on
    // a specified event. This is an important strategy a variety of behaviors
    // are required for various events.
    //
    // *** LATER ***
    // In Spinnaker C, every event handler that is created must be destroyed to avoid
    // memory leaks.
    //
    err = spinDeviceEventHandlerCreate(deviceEventHandler, onDeviceEvent, (void*)eventInfo);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to create event (general). Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Device event created...\n");

    //
    // Register device event handler
    //
    // *** NOTES ***
    // Device event handlers are registered to cameras. If there are multiple cameras,
    // each camera must have the device event handlers registered to it separately.
    // Also, multiple device event handlers may be registered to a single camera.
    //
    // *** LATER ***
    // Device event handlers must be unregistered manually. This must be done prior to
    // releasing the system and while the device event handlers are still in scope.
    //
    if (chosenEvent == GENERIC)
    {
        // Device event handlers registered generally will be triggered type.
        err = spinCameraRegisterDeviceEventHandler(hCam, *deviceEventHandler);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to register device event. Aborting with error %d...\n\n", err);
            return err;
        }

        printf("Device event handler registered generally...\n\n");
    }
    else if (chosenEvent == SPECIFIC)
    {
        // Device events handlers registered to a specific event will only
        // be triggered by the type of event that is registered.
        err = spinCameraRegisterDeviceEventHandlerEx(hCam, *deviceEventHandler, "EventExposureEnd");
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to register device event. Aborting with error %d...\n\n", err);
            return err;
        }

        printf("Device event handler registered specifically to EventExposureEnd events...\n");
    }

    return err;
}

// This function resets the example by unregistering the device event.
spinError ResetDeviceEvents(spinCamera hCam, spinDeviceEventHandler deviceEventHandler)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    //
    // Unregister device event handler
    //
    // *** NOTES ***
    // It is important to unregister all device event handlers from all cameras that
    // they are registered to.
    //
    spinCameraUnregisterDeviceEventHandler(hCam, deviceEventHandler);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to unregister event (general). Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Device event unregistered...\n");

    //
    // Destroy event handlers
    //
    // *** NOTES ***
    // Event handlers must be destroyed in order to avoid memory leaks.
    //
    spinDeviceEventHandlerDestroy(deviceEventHandler);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to destroy device event. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Device event destroyed...\n\n");

    return err;
}

// This function prints the device information of the camera from the transport
// layer; please see NodeMapInfo_C example for more in-depth comments on
// printing device information from the nodemap.
spinError PrintDeviceInfo(spinNodeMapHandle hNodeMap)
{
    spinError err = SPINNAKER_ERR_SUCCESS;
    unsigned int i = 0;

    printf("\n*** DEVICE INFORMATION ***\n\n");

    // Retrieve device information category node
    spinNodeHandle hDeviceInformation = NULL;

    err = spinNodeMapGetNode(hNodeMap, "DeviceInformation", &hDeviceInformation);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve node. Non-fatal error %d...\n\n", err);
        return err;
    }

    // Retrieve number of nodes within device information node
    size_t numFeatures = 0;

    if (IsReadable(hDeviceInformation, "DeviceInformation"))
    {
        err = spinCategoryGetNumFeatures(hDeviceInformation, &numFeatures);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve number of nodes. Non-fatal error %d...\n\n", err);
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
            printf("Unable to retrieve node. Non-fatal error %d...\n\n", err);
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
                printf("Unable to retrieve node type. Non-fatal error %d...\n\n", err);
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

// This function acquires and saves 10 images from a device; please see
// Acquisition_C example for more in-depth comments on the acquisition of
// images.
spinError AcquireImages(spinCamera hCam, spinNodeMapHandle hNodeMap, spinNodeMapHandle hNodeMapTLDevice)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    printf("\n*** IMAGE ACQUISITION ***\n\n");

    // Set acquisition mode to continuous
    spinNodeHandle hAcquisitionMode = NULL;
    spinNodeHandle hAcquisitionModeContinuous = NULL;
    int64_t acquisitionModeContinuous = 0;

    err = spinNodeMapGetNode(hNodeMap, "AcquisitionMode", &hAcquisitionMode);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to set acquisition mode to continuous (node retrieval). Aborting with error %d...\n\n", err);
        return err;
    }

    if (IsReadable(hAcquisitionMode, "AcquisitionMode"))
    {
        err = spinEnumerationGetEntryByName(hAcquisitionMode, "Continuous", &hAcquisitionModeContinuous);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to set acquisition mode to continuous (entry 'continuous' retrieval). Aborting with error "
                "%d...\n\n",
                err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "AcquistionMode 'Continuous'");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    if (IsReadable(hAcquisitionModeContinuous, "AcquisitionModeContinuous"))
    {
        err = spinEnumerationEntryGetIntValue(hAcquisitionModeContinuous, &acquisitionModeContinuous);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to set acquisition mode to continuous (entry int value retrieval). Aborting with error "
                "%d...\n\n",
                err);
            return err;
        }
    }
    else
    {
        PrintRetrieveNodeFailure("entry", "AcquisitionModeContinuous");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // set acquisition mode to continuous
    if (IsWritable(hAcquisitionMode, "AcquisitionMode"))
    {
        err = spinEnumerationSetIntValue(hAcquisitionMode, acquisitionModeContinuous);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf(
                "Unable to set acquisition mode to continuous (entry int value setting). Aborting with error %d...\n\n",
                err);
            return err;
        }
        printf("Acquisition mode set to continuous...\n");
    }
    else
    {
        PrintRetrieveNodeFailure("node", "AcquisitionMode");
        return SPINNAKER_ERR_ACCESS_DENIED;
    }

    // Begin acquiring images
    err = spinCameraBeginAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to begin image acquisition. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Acquiring images...\n");

    // Retrieve device serial number for filename
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

            printf("Device serial number retrieved as %s...\n", deviceSerialNumber);
        }
        else
        {
            PrintRetrieveNodeFailure("node", "DeviceSerialNumber");
            strcpy(deviceSerialNumber, "");
            lenDeviceSerialNumber = 0;
        }
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
        // Retrieve next received image
        spinImage hResultImage = NULL;

        err = spinCameraGetNextImageEx(hCam, 1000, &hResultImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to get next image. Non-fatal error %d...\n\n", err);
            continue;
        }

        // Ensure image completion
        bool8_t isIncomplete = False;
        bool8_t hasFailed = False;

        err = spinImageIsIncomplete(hResultImage, &isIncomplete);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to determine image completion. Non-fatal error %d...\n\n", err);
            hasFailed = True;
        }

        if (isIncomplete)
        {
            spinImageStatus imageStatus = SPINNAKER_IMAGE_STATUS_NO_ERROR;

            err = spinImageGetStatus(hResultImage, &imageStatus);
            if (err != SPINNAKER_ERR_SUCCESS)
            {
                printf("Unable to retrieve image status. Non-fatal error %d...\n\n", err);
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
                printf("Unable to release image. Non-fatal error %d...\n\n", err);
            }

            continue;
        }

        // Print image information
        size_t width = 0;
        size_t height = 0;

        err = spinImageGetWidth(hResultImage, &width);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve image width. Non-fatal error %d...\n\n", err);
        }

        err = spinImageGetHeight(hResultImage, &height);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve image height. Non-fatal error %d...\n\n", err);
        }

        printf("Grabbed image %u, width = %u, height = %u\n", imageCnt, (unsigned int)width, (unsigned int)height);

        // Convert image to mono 8
        spinImage hConvertedImage = NULL;

        err = spinImageCreateEmpty(&hConvertedImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to create image. Non-fatal error %d...\n\n", err);
            hasFailed = True;
        }

        err = spinImageProcessorConvert(hImageProcessor, hResultImage, hConvertedImage, PixelFormat_Mono8);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to convert image. Non-fatal error %d...\n\n", err);
            hasFailed = True;
        }

        // Create unique file name
        char filename[MAX_BUFF_LEN];

        if (lenDeviceSerialNumber == 0)
        {
            sprintf(filename, "DeviceEvents-C-%d.jpg", imageCnt);
        }
        else
        {
            sprintf(filename, "DeviceEvents-C-%s-%d.jpg", deviceSerialNumber, imageCnt);
        }

        // Save image
        err = spinImageSave(hConvertedImage, filename, SPINNAKER_IMAGE_FILE_FORMAT_JPEG);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to save image. Non-fatal error %d...\n\n", err);
        }
        else
        {
            printf("Image saved at %s\n\n", filename);
        }

        // Destroy converted image
        err = spinImageDestroy(hConvertedImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to destroy image. Non-fatal error %d...\n\n", err);
        }

        // Release complete image
        err = spinImageRelease(hResultImage);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to release image. Non-fatal error %d...\n\n", err);
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

    // End Acquisition
    err = spinCameraEndAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to end acquisition. Non-fatal error %d...\n\n", err);
    }

    return err;
}

// This function acts as the body of the example; please see NodeMapInfo_C
// example for more in-depth comments on setting up cameras.
spinError RunSingleCamera(spinCamera hCam)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    // Retrieve TL device nodemap and print device information
    spinNodeMapHandle hNodeMapTLDevice = NULL;

    err = spinCameraGetTLDeviceNodeMap(hCam, &hNodeMapTLDevice);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve TL device nodemap. Non-fatal error %d...\n\n", err);
    }
    else
    {
        err = PrintDeviceInfo(hNodeMapTLDevice);
    }

    // Initialize camera
    err = spinCameraInit(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to initialize camera. Aborting with error %d...\n\n", err);
        return err;
    }

    // Retrieve GenICam nodemap
    spinNodeMapHandle hNodeMap = NULL;

    err = spinCameraGetNodeMap(hCam, &hNodeMap);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve GenICam nodemap. Aborting with error %d...\n\n", err);
        return err;
    }

    // Configure device events
    spinDeviceEventHandler deviceEventHandler = NULL;
    userData eventInfo;

    err = ConfigureDeviceEvents(hNodeMap, hCam, &deviceEventHandler, &eventInfo);
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

    // Reset device events
    err = ResetDeviceEvents(hCam, deviceEventHandler);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return err;
    }

    // Deinitialize camera
    err = spinCameraDeInit(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to deinitialize camera. Non-fatal error %d...\n\n", err);
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
        return -1;
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
        printf("Unable to retrieve system instance. Aborting with error %d...\n\n", err);
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
        printf("Unable to create camera list. Aborting with error %d...\n\n", err);
        return err;
    }

    err = spinSystemGetCameras(hSystem, hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve camera list. Aborting with error %d...\n\n", err);
        return err;
    }

    // Retrieve number of cameras
    size_t numCameras = 0;

    err = spinCameraListGetSize(hCameraList, &numCameras);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve number of cameras. Aborting with error %d...\n\n", err);
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
            printf("Unable to clear camera list. Aborting with error %d...\n\n", err);
            return err;
        }

        err = spinCameraListDestroy(hCameraList);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to destroy camera list. Aborting with error %d...\n\n", err);
            return err;
        }

        // Release system
        err = spinSystemReleaseInstance(hSystem);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to release system instance. Aborting with error %d...\n\n", err);
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
            printf("Unable to retrieve camera from list. Aborting with error %d...\n\n", err);
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
        printf("Unable to clear camera list. Aborting with error %d...\n\n", err);
        return err;
    }

    err = spinCameraListDestroy(hCameraList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to destroy camera list. Aborting with error %d...\n\n", err);
        return err;
    }

    // Release system
    err = spinSystemReleaseInstance(hSystem);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to release system instance. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("\nDone! Press Enter to exit...\n");
    getchar();

    return errReturn;
}
