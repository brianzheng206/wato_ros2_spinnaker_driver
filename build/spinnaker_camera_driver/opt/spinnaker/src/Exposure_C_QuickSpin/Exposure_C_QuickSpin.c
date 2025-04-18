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
 *  @example Exposure_C_Quickspin.c
 *
 *  @brief Exposure_C_Quickspin.c shows how to customize image exposure
 *  time using the QuickSpin API. QuickSpin is a subset of the Spinnaker library
 *  that allows for simpler node access and control.
 *
 *  This example prepares the camera, sets a new exposure time, and restores
 *  the camera to its default state. Ensure custom values fall within an
 *  acceptable range is also touched on. Retrieving and setting node values
 *  is the only portion of hte example that differs from Exposure_C.
 *
 *  A much wider range of topics is covered in the full Spinnaker examples than
 *  in the QuickSpin ones. There are only enough QuickSpin examples to
 *  demonstrate node access and to get started with the API; please see full
 *  Spinnaker examples for further or specific knowledge on a topic.
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

// This function configures a custom exposure time. Automatic exposure is turned
// off in order to allow for the customization, and then the custom setting is
// applied.
spinError ConfigureExposure(quickSpin qs)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    printf("\n\n*** CONFIGURING EXPOSURE ***\n\n");

    //
    // Turn off automatic exposure mode
    //
    // *** NOTES ***
    // Automatic exposure prevents the manual configuration of exposure time
    // and needs to be turned off. Enumerations representing entry have been
    // added to QuickSpin. This allows for the much easier setting of
    // enumeration nodes to new values.
    //
    // In C, the naming convention of QuickSpin enums is the name of the
    // enumeration node followed by an underscore and the symbolic of the entry
    // node. Selecting "Off" on the "ExposureAuto" node is thus named
    // "ExposureAuto_Off".
    //
    // *** LATER ***
    // Exposure time can be set automatically or manually as needed. This
    // example turns automatic exposure off to set it manually and back on to
    // return the camera to its default state.
    //
    err = spinEnumerationSetEnumValue(qs.ExposureAuto, ExposureAuto_Off);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to disable automatic exposure. Aborting with error %d...\n", err);
        return err;
    }

    printf("Automatic exposure disabled...\n");

    //
    // Set exposure time manually; exposure time recorded in microseconds
    //
    // *** NOTES ***
    // It is ensured that the desired exposure time does not exceed the maximum.
    // Exposure time is counted in microseconds - this can be found out either
    // by retrieving the unit with the spinFloatGetUnit() methods or by
    // checking SpinView.
    //
    double exposureTimeMax = 0.0;
    double exposureTimeToSet = 2000000.0;

    // Retrieve maximum value
    err = spinFloatGetMax(qs.ExposureTime, &exposureTimeMax);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to set exposure time. Aborting with error %d...\n", err);
        return err;
    }

    // Ensure desired exposure time does not exceed maximum
    if (exposureTimeToSet > exposureTimeMax)
    {
        exposureTimeToSet = exposureTimeMax;
    }

    // Set desired exposure time as new value
    err = spinFloatSetValue(qs.ExposureTime, exposureTimeToSet);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to set exposure time. Aborting with error %d...\n", err);
        return err;
    }

    printf("Exposure time set to %f us...\n", exposureTimeToSet);

    return err;
}

spinError ResetExposure(quickSpin qs)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    //
    // Turn automatic exposure back on
    //
    // *** NOTES ***
    // It is recommended to have automatic exposure enabled whenever manual
    // exposure settings are not required.
    //
    err = spinEnumerationSetIntValue(qs.ExposureAuto, ExposureAuto_Continuous);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to enable automatic exposure. Aborting with error %d...\n", err);
        return err;
    }

    printf("Automatic exposure enabled...\n\n");

    return err;
}

// This function prints the device information of the camera from the transport
// layer; please see NodeMapInfo_C example for more in-depth comments on
// printing device information from the nodemap.
spinError PrintDeviceInfo(spinCamera hCamera)
{
    spinError err = SPINNAKER_ERR_SUCCESS;
    unsigned int i = 0;

    printf("\n*** DEVICE INFORMATION ***\n\n");

    // Retrieve nodemap
    spinNodeMapHandle hNodeMap = NULL;

    err = spinCameraGetTLDeviceNodeMap(hCamera, &hNodeMap);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve nodemap. Non-fatal error %d...\n\n", err);
        return err;
    }

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

    err = spinCategoryGetNumFeatures(hDeviceInformation, &numFeatures);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve number of nodes. Non-fatal error %d...\n\n", err);
        return err;
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

        err = spinNodeGetType(hFeatureNode, &featureType);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve node type. Non-fatal error %d...\n\n", err);
            continue;
        }

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
spinError AcquireImages(spinCamera hCam, quickSpin qs, quickSpinTLDevice qsD)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    printf("\n\n*** IMAGE ACQUISITION ***\n\n");

    // Set acquisition mode to continuous
    err = spinEnumerationSetEnumValue(qs.AcquisitionMode, AcquisitionMode_Continuous);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf(
            "Unable to set acquisition mode to continuous (entry int value setting). Aborting with error %d...\n\n",
            err);
        return err;
    }

    printf("Acquisition mode set to continuous...\n");

    // Begin acquiring images
    err = spinCameraBeginAcquisition(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to begin image acquisition. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Acquiring images...\n");

    // Retrieve device serial number for filename
    char deviceSerialNumber[MAX_BUFF_LEN];
    size_t lenDeviceSerialNumber = MAX_BUFF_LEN;

    err = spinStringGetValue(qsD.DeviceSerialNumber, deviceSerialNumber, &lenDeviceSerialNumber);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        strcpy(deviceSerialNumber, "");
        lenDeviceSerialNumber = 0;
    }
    else
    {
        printf("Device serial number retrieved as %s...\n", deviceSerialNumber);
    }
    printf("\n");

    double exposureTime;
    uint64_t timeout;

    // Get the value of exposure time to set an appropriate timeout for GetNextImage
    err = spinFloatGetValue(qs.ExposureTime, &exposureTime);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to read exposure time. Aborting with error %d...\n", err);
        return err;
    }
    else
    {
        // The exposure time is retrieved in µs so it needs to be converted to ms to keep consistency with the unit
        // being used in GetNextImage
        timeout = (uint64_t)(exposureTime / 1000 + 1000);
    }

    // Retrieve, convert, and save images
    const unsigned int k_numImages = 5;
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

        // Retrieve next received image and ensure image completion
        // By default, GetNextImage will block indefinitely until an image arrives.
        // In this example, the timeout value is set to [exposure time + 1000]ms to ensure that an image has enough
        // time to arrive under normal conditions
        err = spinCameraGetNextImageEx(hCam, timeout, &hResultImage);
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
            printf("Unable to retrieve image width. Non-fatal error %d...\n", err);
        }

        err = spinImageGetHeight(hResultImage, &height);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to retrieve image height. Non-fatal error %d...\n", err);
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
            sprintf(filename, "Exposure-C-%d.jpg", imageCnt);
        }
        else
        {
            sprintf(filename, "Exposure-C-%s-%d.jpg", deviceSerialNumber, imageCnt);
        }

        // Save image
        err = spinImageSave(hConvertedImage, filename, SPINNAKER_IMAGE_FILE_FORMAT_JPEG);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            printf("Unable to save image. Non-fatal error %d...\n", err);
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

        // Release image
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

    // Print device information
    err = PrintDeviceInfo(hCam);

    // Initialize camera
    err = spinCameraInit(hCam);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to initialize camera. Aborting with error %d...\n\n", err);
        return err;
    }

    // Pre-fetch TL device nodes
    quickSpinTLDevice qsD;

    err = quickSpinTLDeviceInit(hCam, &qsD);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to pre-fetch TL device nodes. Aborting with error %d...\n\n", err);
        return err;
    }

    // Pre-fetch GenICam nodes
    quickSpin qs;

    err = quickSpinInit(hCam, &qs);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to pre-fetch GenICam nodes. Aborting with error %d...\n\n", err);
        return err;
    }

    // Configure exposure
    err = ConfigureExposure(qs);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return err;
    }

    // Acquire images
    err = AcquireImages(hCam, qs, qsD);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        return err;
    }

    // Reset exposure
    err = ResetExposure(qs);
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
