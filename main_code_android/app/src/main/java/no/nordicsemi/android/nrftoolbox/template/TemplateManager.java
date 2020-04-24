/*
 * Copyright (c) 2015, Nordic Semiconductor
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
package no.nordicsemi.android.nrftoolbox.template;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.Context;
import androidx.annotation.NonNull;
import android.util.Log;

import java.util.Calendar;
import java.util.UUID;

import no.nordicsemi.android.ble.data.Data;
import no.nordicsemi.android.log.LogContract;
import no.nordicsemi.android.nrftoolbox.parser.TemplateParser;
import no.nordicsemi.android.nrftoolbox.profile.LoggableBleManager;
import no.nordicsemi.android.nrftoolbox.template.callback.SensDataCallback;
import no.nordicsemi.android.nrftoolbox.template.callback.StatDataCallback;
import no.nordicsemi.android.nrftoolbox.template.callback.ContDataCallback;

/**
 * Modify to template manager to match your requirements.
 */
public class TemplateManager extends LoggableBleManager<TemplateManagerCallbacks> {

	// TODO Replace the services and characteristics below to match your device.
	//The Generic Access service UUID and the Device Name characteristic UUID, just to to aware of their existence:
	private static final UUID UUID_SERVICE_GENERIC_ACCESS     = UUID.fromString("00001800-0000-1000-8000-00805f9b34fb");
	private static final UUID UUID_CHARACTERISTIC_DEVICE_NAME = UUID.fromString("00002A00-0000-1000-8000-00805f9b34fb");

	//The custom STAT service and characteristic UUIDs:
	public static final UUID UUID_SERVICE_STAT         = UUID.fromString("e2531400-ffaf-313f-a94f-f4b934ae79ab");
	private static final UUID UUID_CHARACTERISTIC_STAT = UUID.fromString("e2531401-ffaf-313f-a94f-f4b934ae79ab");

    //The custom SENS service and characteristic UUIDs:
    public static final UUID UUID_SERVICE_SENS         = UUID.fromString("f3641400-00b0-4240-ba50-05ca45bf8abd");
    private static final UUID UUID_CHARACTERISTIC_SENS = UUID.fromString("f3641401-00b0-4240-ba50-05ca45bf8abd");

	//The custom CONT service and characteristic UUIDs:
	public static final UUID UUID_SERVICE_CONT         = UUID.fromString("04751400-11c1-5351-cb61-16db56c09bce");
	private static final UUID UUID_CHARACTERISTIC_CONT = UUID.fromString("04751401-11c1-5351-cb61-16db56c09bce");


	// TODO Add more services and characteristics references.
	private BluetoothGattCharacteristic characteristicDeviceName;
	private BluetoothGattCharacteristic characteristicStat;
    private BluetoothGattCharacteristic characteristicSens;
	private BluetoothGattCharacteristic characteristicCont;

	public TemplateManager(final Context context) {
		super(context);
	}

	@NonNull
	@Override
	protected BleManagerGattCallback getGattCallback() {
		return new TemplateManagerGattCallback();
	}

	/**
	 * The callback for detecting updates of the Stat Characteristic.
	 */
	private StatDataCallback mStatDataCallback = new StatDataCallback() {
		@Override
		public void onDataReceived(@NonNull final BluetoothDevice device, @NonNull final Data data) {
			log(LogContract.Log.Level.APPLICATION, TemplateParser.parse(data));
			super.onDataReceived(device, data);
		}

		@Override
		public void onCharacteristicStatUpdate(@NonNull final BluetoothDevice device, final int[] dataArray) {
			// Let's lass received data to the service
			callbacks.onCharacteristicStatUpdate(device, dataArray);
		}

		@Override
		public void onInvalidDataReceived(@NonNull final BluetoothDevice device, @NonNull final Data data) {
			log(Log.WARN, "Invalid data received: " + data);
		}
	};

	/**
	 * The callback for detecting updates of the Sens Characteristic.
	 */
	private SensDataCallback mSensDataCallback = new SensDataCallback() {
		@Override
		public void onDataReceived(@NonNull final BluetoothDevice device, @NonNull final Data data) {
			log(LogContract.Log.Level.APPLICATION, TemplateParser.parse(data));
			super.onDataReceived(device, data);
		}

		@Override
		public void onCharacteristicSensUpdate(@NonNull final BluetoothDevice device, final int[] dataArray) {
			// Let's lass received data to the service
			callbacks.onCharacteristicSensUpdate(device, dataArray);
		}

		@Override
		public void onInvalidDataReceived(@NonNull final BluetoothDevice device, @NonNull final Data data) {
			log(Log.WARN, "Invalid data received: " + data);
		}
	};

	/**
	 * The callback for detecting updates of the Cont Characteristic.
	 */
	private ContDataCallback mContDataCallback = new ContDataCallback() {
		@Override
		public void onDataReceived(@NonNull final BluetoothDevice device, @NonNull final Data data) {
			log(LogContract.Log.Level.APPLICATION, TemplateParser.parse(data));
			super.onDataReceived(device, data);
		}

		@Override
		public void onCharacteristicContUpdate(@NonNull final BluetoothDevice device, final int[] dataArray) {
			// Let's lass received data to the service
			callbacks.onCharacteristicContUpdate(device, dataArray);
		}

		@Override
		public void onInvalidDataReceived(@NonNull final BluetoothDevice device, @NonNull final Data data) {
			log(Log.WARN, "Invalid data received: " + data);
		}
	};

	/**
	 * BluetoothGatt callbacks for connection/disconnection, service discovery,
	 * receiving indication, etc.
	 */
	private class TemplateManagerGattCallback extends BleManagerGattCallback {

		@Override
		protected void initialize() {
			// Initialize the Battery Manager. It will enable Battery Level notifications.
			// Remove it if you don't need this feature.
			super.initialize();

			// TODO Initialize your manager here.
			// Initialization is done once, after the device is connected. Usually it should
			// enable notifications or indications on some characteristics, write some data or
			// read some features / version.
			// After the initialization is complete, the onDeviceReady(...) method will be called.

			// Increase the MTU
			requestMtu(43)
					.with((device, mtu) -> log(LogContract.Log.Level.APPLICATION, "MTU changed to " + mtu))
					.done(device -> {
						// You may do some logic in here that should be done when the request finished successfully.
						// In case of MTU this method is called also when the MTU hasn't changed, or has changed
						// to a different (lower) value. Use .with(...) to get the MTU value.
					})
					.fail((device, status) -> log(Log.WARN, "MTU change not supported"))
					.enqueue();

			// Set notification callback
			setNotificationCallback(characteristicStat)
					// This callback will be called each time the notification is received
					.with(mStatDataCallback);

			// Enable notifications
			enableNotifications(characteristicStat)
					// Method called after the data were sent (data will contain 0x0100 in this case)
					.with((device, data) -> log(Log.DEBUG, "Data sent: " + data))
					// Method called when the request finished successfully. This will be called after .with(..) callback
					.done(device -> log(LogContract.Log.Level.APPLICATION, "Stat Notifications enabled successfully"))
					// Methods called in case of an error, for example when the characteristic does not have Notify property
					.fail((device, status) -> log(Log.WARN, "Failed to enable Stat notifications"))
					.enqueue();


			// Set notification callback
			setNotificationCallback(characteristicSens)
					// This callback will be called each time the notification is received
					.with(mSensDataCallback);

			// Enable notifications
			enableNotifications(characteristicSens)
					// Method called after the data were sent (data will contain 0x0100 in this case)
					.with((device, data) -> log(Log.DEBUG, "Data sent: " + data))
					// Method called when the request finished successfully. This will be called after .with(..) callback
					.done(device -> log(LogContract.Log.Level.APPLICATION, "Sens Notifications enabled successfully"))
					// Methods called in case of an error, for example when the characteristic does not have Notify property
					.fail((device, status) -> log(Log.WARN, "Failed to enable Sens notifications"))
					.enqueue();

			// Set notification callback
			setNotificationCallback(characteristicCont)
					// This callback will be called each time the notification is received
					.with(mContDataCallback);

			// Enable notifications
			enableNotifications(characteristicCont)
					// Method called after the data were sent (data will contain 0x0100 in this case)
					.with((device, data) -> log(Log.DEBUG, "Data sent: " + data))
					// Method called when the request finished successfully. This will be called after .with(..) callback
					.done(device -> log(LogContract.Log.Level.APPLICATION, "Cont Notifications enabled successfully"))
					// Methods called in case of an error, for example when the characteristic does not have Notify property
					.fail((device, status) -> log(Log.WARN, "Failed to enable Cont notifications"))
					.enqueue();

		}

		@Override
		protected boolean isRequiredServiceSupported(@NonNull final BluetoothGatt gatt) {
			// TODO Initialize required characteristics.
			// It should return true if all has been discovered (that is that device is supported).
			final BluetoothGattService serviceGenericAccess = gatt.getService(UUID_SERVICE_GENERIC_ACCESS);
			if (serviceGenericAccess != null) {
				characteristicDeviceName = serviceGenericAccess.getCharacteristic(UUID_CHARACTERISTIC_DEVICE_NAME);
			}
			final BluetoothGattService serviceStat = gatt.getService(UUID_SERVICE_STAT);
			if (serviceStat != null) {
				characteristicStat = serviceStat.getCharacteristic(UUID_CHARACTERISTIC_STAT);
			}
			final BluetoothGattService serviceSens = gatt.getService(UUID_SERVICE_SENS);
			if (serviceSens != null) {
				characteristicSens = serviceSens.getCharacteristic(UUID_CHARACTERISTIC_SENS);
			}
			final BluetoothGattService serviceCont = gatt.getService(UUID_SERVICE_CONT);
			if (serviceCont != null) {
				characteristicCont = serviceCont.getCharacteristic(UUID_CHARACTERISTIC_CONT);
			}
			return characteristicDeviceName != null && characteristicStat != null && characteristicSens != null && characteristicCont != null;
		}

		@Override
		protected void onDeviceDisconnected() {

			// TODO Release references to your characteristics.
			characteristicDeviceName = null;
			characteristicStat = null;
			characteristicSens = null;
			characteristicCont = null;

		}

		@Override
		protected void onDeviceReady() {
			super.onDeviceReady();

			// Initialization is now ready.
			// The service or activity has been notified with TemplateManagerCallbacks#onDeviceReady().
			// TODO Do some extra logic here, of remove onDeviceReady().

			// Device is ready, let's read something here. Usually there is nothing else to be done
			// here, as all had been done during initialization.
			readCharacteristicStat();

		}
	}

	// TODO Define manager's API

	/**
	 * This method will read important data from the device.
	 *
	 * @param parameter parameter to be written. (not used)
	 */

	//This value stores the last updated STAT characteristic from the nRF52:
	public static final int LENGTH_DATA_DEVICE_STATUS = 13;
	private byte dataDeviceStatus[] = new byte[LENGTH_DATA_DEVICE_STATUS];

	void readCharacteristicStat() {
		readCharacteristic(characteristicStat).with(mStatDataCallback).enqueue();
	}

	/**
	 * This method will write important data to the device.
	 */
	void sendCommandFromPhone(int[] durationTime) {

        readCharacteristic(characteristicStat).with((device, data) -> {
            // If the read value has more than one byte, then:
            if (data.size() > 0) {

                // Read STAT characteristic from phone and update internal variables:
                for(int i = 0; i < dataDeviceStatus.length; i++){
                    dataDeviceStatus[i] = (byte)(data.getIntValue(Data.FORMAT_UINT8, i) & 0xFF);
                }
                Log.v("readCharacteristicStat", "dataDeviceStatus[0] = " + dataDeviceStatus[0]);

                // Toggle the commandFromPhone:
                byte commandFromPhone = (dataDeviceStatus[0] == 1)?((byte)0):((byte)1);
                Log.v("sendCommandFromPhone", "commandFromPhone = " + commandFromPhone);

                ///////////////////////////////////////////////////////////////////
                // Change data to send to the STAT characteristic:
                byte[] dataDeviceStatusRequest = new byte[LENGTH_DATA_DEVICE_STATUS];
                dataDeviceStatusRequest[0] = commandFromPhone;
                for(int i = 1; i < dataDeviceStatus.length; i++){
                    dataDeviceStatusRequest[i] = dataDeviceStatus[i];
                }

                // If the is no data stored on flash, then create a file name:
                if(dataDeviceStatus[2] == 0){
					Calendar now = Calendar.getInstance();
					dataDeviceStatusRequest[3] = (byte) (now.get(Calendar.YEAR) - 2000);
					dataDeviceStatusRequest[4] = (byte) (now.get(Calendar.MONTH) + 1); // Note: zero based!
					dataDeviceStatusRequest[5] = (byte) now.get(Calendar.DAY_OF_MONTH);
					dataDeviceStatusRequest[6] = (byte) now.get(Calendar.HOUR_OF_DAY);
					dataDeviceStatusRequest[7] = (byte) now.get(Calendar.MINUTE);
					dataDeviceStatusRequest[8] = (byte) now.get(Calendar.SECOND);
					dataDeviceStatusRequest[9]  = (byte) durationTime[0];
					dataDeviceStatusRequest[10] = (byte) durationTime[1];
					dataDeviceStatusRequest[11] = (byte) durationTime[2];
					dataDeviceStatusRequest[12] = (byte) 37;
				}
                ///////////////////////////////////////////////////////////////////

                // Write STAT characteristic:
                writeCharacteristic(characteristicStat, dataDeviceStatusRequest)
                        // If data are longer than MTU-3, they will be chunked into multiple packets.
                        // Check out other split options, with .split(...).
                        .split()
                        // Callback called when data were sent, or added to outgoing queue in case
                        // Write Without Request type was used.
                        .with((mdevice, mdata) -> Log.v("writeCharacteristicStat", mdata.getByte(0) + "," + mdata.getByte(1) + "," + mdata.getByte(2) + "," + mdata.getByte(3) + "," + mdata.getByte(4) + "," + mdata.getByte(5) + "," + mdata.getByte(6) + "," + mdata.getByte(7) + "," + mdata.getByte(8) + " bytes were sent"))
                        // Callback called when data were sent, or added to outgoing queue in case
                        // Write Without Request type was used. This is called after .with(...) callback.
                        .done(mdevice -> Log.v("writeCharacteristicStat", "Write to STAT characteristic Successful."))
                        // Callback called when write has failed.
                        .fail((mdevice, status) -> Log.v("writeCharacteristicStat", "Failed to write STAT characteristic."))
                        .enqueue();

            }
            // If the read value hasn' data:
            else {
                Log.v("readCharacteristicStat", "Value to read is empty!");
            }
        }).enqueue();

	}

}
