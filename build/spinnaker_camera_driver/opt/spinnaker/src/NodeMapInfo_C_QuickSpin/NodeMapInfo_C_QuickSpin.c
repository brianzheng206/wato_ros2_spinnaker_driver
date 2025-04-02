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
 *  @example NodeMapInfo_C_QuickSpin.c
 *
 *  @brief NodeMapInfo_C_QuickSpin.c shows how to interact with nodes
 *  using the QuickSpin API. QuickSpin is a subset of the Spinnaker library
 *  that allows for simpler node access and control.
 *
 *  This example demonstrates the retrieval of information from both the
 *  transport layer and the camera. Because the focus of this example is node
 *  access, which is where QuickSpin and regular Spinnaker differ, this
 *  example differs from NodeMapInfo_C quite a bit.
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

// This macro defines the maximum number of characters that will be printed out
// for any information retrieved from a node.
#define MAX_CHARS 35

// This function prints node information if applicable.
void PrintNodeInfo(spinNodeHandle spinNodeHandler)
{
    //
    // Notice that each node is checked for availability and readability
    // prior to value retrieval. Checking for availability and readability
    // (or writability when applicable) whenever a node is accessed is
    // important in terms of error handling. If a node retrieval error
    // occurs but remains unhandled, an exception is thrown.
    //
    if (spinNodeHandler != NULL)
    {
        spinError err = SPINNAKER_ERR_SUCCESS;
        bool8_t isNodeInfoReadable = False;
        err = spinNodeIsReadable(spinNodeHandler, &isNodeInfoReadable);
        if (err == SPINNAKER_ERR_SUCCESS)
        {
            char nodeInfoItem[MAX_BUFF_LEN];
            size_t lenNodeInfoItem = MAX_BUFF_LEN;

            err = spinNodeToString(spinNodeHandler, nodeInfoItem, &lenNodeInfoItem);
            if (err == SPINNAKER_ERR_SUCCESS)
            {
                printf("%s\n", nodeInfoItem);
                return;
            }
            else
            {
                printf("Unable to convert node information due to error %d...\n", err);
                return;
            }
        }
    }

    printf("unavailable\n");
}

// This function prints device information from the transport layer.
spinError PrintTransportLayerDeviceInfo(spinCamera hCamera)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    //
    // Pre-fetch TL device nodes
    //
    // *** NOTES ***
    // In order to use QuickSpin in C, it is required to manually pre-fetch
    // all available nodes into a QuickSpin struct, which then acts as the
    // means of accessing its nodes. In order to pre-fetch TL device nodes,
    // the quickSpinTLDeviceInit() function and its corresponding
    // quickSpinTLDevice object are used.
    //
    // The main advantage of transport layer nodes is that they do not require
    // camera initialization. The main disadvantage is that there is much less
    // camera information and interaction that may be performed.
    //
    quickSpinTLDevice qsD;

    err = quickSpinTLDeviceInit(hCamera, &qsD);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to pre-fetch TL device nodes. Aborting with error %d...\n\n", err);
        return err;
    }

    //
    // Print device information from the transport layer
    //
    // *** NOTES ***
    // In QuickSpin, accessing device information on the transport layer is
    // accomplished via a camera's quickSpinTLDevice object. The
    // quickSpinTLDevice object houses nodes related to general device
    // information such as the three demonstrated below, device access status,
    // XML and GUI paths and locations, and GEV information to name a few. The
    // quickSpinTLDevice property allows access to nodes that would generally
    // be retrieved through the TL device nodemap in full Spinnaker.
    //

    // Print device serial number
    printf("Device serial number: ");
    PrintNodeInfo(qsD.DeviceSerialNumber);

    // Print device vendor name
    printf("Device vendor name: ");
    PrintNodeInfo(qsD.DeviceVendorName);

    // Print device display name
    printf("Device display name: ");
    PrintNodeInfo(qsD.DeviceDisplayName);

    printf("\n");
    return err;
}

// This function prints stream information from the transport layer.
spinError PrintTransportLayerStreamInfo(spinCamera hCamera)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    //
    // Pre-fetch TL device nodes
    //
    // *** NOTES ***
    // Separate from TL device, but still on the transport layer, the TL stream
    // nodes provide information. TL stream nodes are accessed via the
    // quickspinTLStreamInit() function and its corresponding quickSpinTLStream
    // object.
    //
    quickSpinTLStream qsS;

    err = quickSpinTLStreamInit(hCamera, &qsS);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to pre-fetch TL stream nodes. Aborting with error %d...\n\n", err);
        return err;
    }

    //
    // Print stream information from the transport layer
    //
    // *** NOTES ***
    // In QuickSpin, accessing stream information on the transport layer is
    // accomplished via a camera's quickSpinTLStream object. The
    // quickSpinTLStream object houses nodes related to streaming such as the
    // two demonstrated below, buffer information, and GEV packet information
    // to name a few. The quickSpinTLStream object allows access to nodes that
    // would generally be retrieved through the TL stream nodemap in full
    // Spinnaker.
    //
    // Print stream serial
    printf("Stream ID: ");
    PrintNodeInfo(qsS.StreamID);

    // Print stream type
    printf("Stream type: ");
    PrintNodeInfo(qsS.StreamType);

    printf("\n");
    return err;
}

// This function prints information about the interface.
spinError PrintTransportLayerInterfaceInfo(spinInterface hInterface)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    //
    // Pre-fetch TL interface nodes
    //
    // *** NOTES ***
    // The final group of nodes on the transport layer are the interface nodes.
    // TL interface nodes are accessed via the quickspinTLInterfaceInit()
    // function and its corresponding quickSpinTLInterface object. Of course,
    // these nodes are accessed via interfaces instead of cameras.
    //
    quickSpinTLInterface qsI;

    err = quickSpinTLInterfaceInit(hInterface, &qsI);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to pre-fetch TL interface nodes. Aborting with error %d...\n\n", err);
        return err;
    }

    //
    // Print interface information from the transport layer
    //
    // *** NOTES ***
    // In QuickSpin, accessing interface information is accomplished via an
    // interface's quickSpinTLInterface object. The quickSpinTLInterface object
    // houses nodes that hold information about the interface such as the three
    // demonstrated below, other general interface information, and
    // GEV addressing information. The quickSpinTLInterface object allows
    // access to nodes that would generally be retrieved through the interface
    // nodemap in full Spinnaker.
    //

    // Print interface display name
    printf("Interface display name: ");
    PrintNodeInfo(qsI.InterfaceDisplayName);

    // Print interface ID
    printf("Interface ID: ");
    PrintNodeInfo(qsI.InterfaceID);

    // Print interface type
    printf("Interface type: ");
    PrintNodeInfo(qsI.InterfaceType);

    //
    // Print host adapter information from the transport layer
    //
    // *** NOTES ***
    // Host adapter information is part of the interface information from the
    // transport layer. In QuickSpin, accessing the host adapter information is
    // also accomplished via the interface's quickSpinTLInterface object. This
    // information can help in determining which interface to use for better
    // performance as some host adapters may have more significant physical
    // limitations.
    //
    // Interface nodes should also always be checked for availability and
    // readability (or writability when applicable). If a node retrieval
    // error occurs but remains unhandled, an exception is thrown.
    //

    // Print host adapter name
    printf("Host adapter name: ");
    PrintNodeInfo(qsI.HostAdapterName);

    // Print host adapter vendor
    printf("Host adapter vendor: ");
    PrintNodeInfo(qsI.HostAdapterVendor);

    // Print host adapter driver version
    printf("Host adapter driver version: ");
    PrintNodeInfo(qsI.HostAdapterDriverVersion);

    printf("\n");
    return err;
}

// This function prints device information from the GenICam nodemap.
spinError PrintGenICamInfo(spinCamera hCamera)
{
    spinError err = SPINNAKER_ERR_SUCCESS;

    //
    // Pre-fetch GenICam nodes
    //
    // *** NOTES ***
    // Unlike other nodes, GenICam nodes are not on the transport layer.
    // These nodes are accessed via the quickSpinInit() function and its
    // corresponding quickSpin object.
    //
    // The main advantages of GenICam nodes is that there are a lot more of
    // them and that they provide a much deeper level of interaction with a
    // camera. The main disadvantage is that they require camera
    // initialization.
    //
    quickSpin qs;

    err = quickSpinInit(hCamera, &qs);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to pre-fetch TL device nodes. Aborting with error %d...\n\n", err);
        return err;
    }

    //
    // Print device information from the camera
    //
    // *** NOTES ***
    // In QuickSpin, accessing most camera information and customizing most
    // settings requires access to GenICam nodes, which requires camera
    // initialization. Note that QuickSpin initialization and camera
    // initialization are two different processes, both of which are required
    // to access GenICam nodes through QuickSpin.
    //
    // Print exposure time
    printf("Exposure time: ");
    PrintNodeInfo(qs.ExposureTime);

    // Print black level
    printf("Black level: ");
    PrintNodeInfo(qs.BlackLevel);

    // Print height
    printf("Height: ");
    PrintNodeInfo(qs.Height);

    printf("\n");
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

    //
    // Retrieve singleton reference to system object
    //
    // *** NOTES ***
    // Everything originates from the system. Notice that it is implemented as
    // a singleton object, making it impossible to have more than one system.
    //
    // *** LATER ***
    // The system object should be cleared prior to program completion. If not
    // released explicitly, it will release itself automatically.
    //
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

    // Retrieve list of interfaces from the system
    spinInterfaceList hInterfaceList = NULL;

    err = spinInterfaceListCreateEmpty(&hInterfaceList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to create interface list. Aborting with error %d...\n\n", err);
        return err;
    }

    err = spinSystemGetInterfaces(hSystem, hInterfaceList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve interface list. Aborting with error %d...\n\n", err);
        return err;
    }

    // Retrieve number of interfaces
    size_t numInterfaces = 0;

    err = spinInterfaceListGetSize(hInterfaceList, &numInterfaces);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to retrieve number of interfaces. Aborting with error %d...\n\n", err);
        return err;
    }

    printf("Number of interfaces detected: %u\n\n", (unsigned int)numInterfaces);

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

    //
    // Print information on each interface
    //
    // *** NOTES ***
    // All USB 3 Vision and GigE Vision interfaces should enumerate for
    // Spinnaker.
    //
    printf("\n*** PRINTING INTERFACE INFORMATION ***\n\n");

    for (i = 0; i < numInterfaces; i++)
    {
        // Retrieve interface
        spinInterface hInterface = NULL;

        err = spinInterfaceListGet(hInterfaceList, i, &hInterface);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = err;
        }

        // Print TL information
        err = PrintTransportLayerInterfaceInfo(hInterface);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = err;
        }

        // Release interface
        err = spinInterfaceRelease(hInterface);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = err;
        }
    }

    //
    // Print general device information on each camera from transport layer
    //
    // *** NOTES ***
    // Transport layer nodes do not require initialization in order to interact
    // with them.
    //
    printf("\n*** PRINTING TRANSPORT LAYER DEVICE INFORMATION ***\n\n");

    for (i = 0; i < numCameras; i++)
    {
        // Retrieve camera
        spinCamera hCamera = NULL;

        err = spinCameraListGet(hCameraList, i, &hCamera);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = err;
        }

        // Print TL device information
        err = PrintTransportLayerDeviceInfo(hCamera);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = err;
        }

        // Release camera
        err = spinCameraRelease(hCamera);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = err;
        }
    }

    //
    // Print streaming information on each camera from transport layer
    //
    // *** NOTES ***
    // Again, initialization is not required to print information from the
    // transport layer; this is equally true of streaming information.
    //
    printf("\n*** PRINTING TRANSPORT LAYER STREAM INFORMATION ***\n\n");

    for (i = 0; i < numCameras; i++)
    {
        // Select camera
        spinCamera hCamera = NULL;

        err = spinCameraListGet(hCameraList, i, &hCamera);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = err;
        }

        // Print TL stream information
        err = PrintTransportLayerStreamInfo(hCamera);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = err;
        }

        // Release camera
        err = spinCameraRelease(hCamera);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = err;
        }
    }

    //
    // Print device information on each camera from GenICam nodemap
    //
    // *** NOTES ***
    // GenICam nodes require initialization in order to interact with
    // them; as such, this loop initializes the camera, prints some information
    // from the GenICam nodemap, and then deinitializes it. If the camera were
    // not initialized, node availability would fail.
    //
    printf("\n*** PRINTING GENICAM INFORMATION ***\n\n");

    for (i = 0; i < numCameras; i++)
    {
        // Select camera
        spinCamera hCamera = NULL;

        err = spinCameraListGet(hCameraList, i, &hCamera);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = err;
        }

        // Initialize camera
        err = spinCameraInit(hCamera);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = err;
        }

        // Print GenICam information
        err = PrintGenICamInfo(hCamera);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = err;
        }

        // Deinitialize information
        err = spinCameraDeInit(hCamera);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = err;
        }

        // Release camera
        err = spinCameraRelease(hCamera);
        if (err != SPINNAKER_ERR_SUCCESS)
        {
            errReturn = err;
        }
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

    // Clear interface list before releasing system
    err = spinInterfaceListClear(hInterfaceList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to clean interface list. Aborting with error %d...\n\n", err);
        return err;
    }

    err = spinInterfaceListDestroy(hInterfaceList);
    if (err != SPINNAKER_ERR_SUCCESS)
    {
        printf("Unable to destroy interface list. Aborting with error %d...\n\n", err);
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
