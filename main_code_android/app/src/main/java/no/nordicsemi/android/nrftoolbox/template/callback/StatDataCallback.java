package no.nordicsemi.android.nrftoolbox.template.callback;

import android.bluetooth.BluetoothDevice;
import androidx.annotation.NonNull;

import no.nordicsemi.android.ble.callback.profile.ProfileDataCallback;
import no.nordicsemi.android.ble.data.Data;

import static no.nordicsemi.android.nrftoolbox.template.TemplateManager.LENGTH_DATA_DEVICE_STATUS;

/**
 * This is a sample data callback, that's based on Heart Rate Measurement characteristic.
 * It parses the HR value and ignores other optional data for simplicity.
 * Check {@link no.nordicsemi.android.ble.common.callback.hr.HeartRateMeasurementDataCallback}
 * for full implementation.
 *
 * TODO Modify the content to parse your data.
 */
@SuppressWarnings("ConstantConditions")
public abstract class StatDataCallback implements ProfileDataCallback, StatCharacteristicCallback {

	@Override
	public void onDataReceived(@NonNull final BluetoothDevice device, @NonNull final Data data) {

		final int[] dataArray = new int[LENGTH_DATA_DEVICE_STATUS];
		for (int i = 0; i < dataArray.length; i++){
			dataArray[i] = data.getIntValue(Data.FORMAT_UINT8, i);
		}

		// Report the parsed value(s)
		onCharacteristicStatUpdate(device, dataArray);

	}
	
}
