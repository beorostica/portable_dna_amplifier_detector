package no.nordicsemi.android.nrftoolbox.template.callback;

import android.bluetooth.BluetoothDevice;
import androidx.annotation.NonNull;

import no.nordicsemi.android.ble.callback.profile.ProfileDataCallback;
import no.nordicsemi.android.ble.data.Data;

/**
 * This is a sample data callback, that's based on Heart Rate Measurement characteristic.
 * It parses the HR value and ignores other optional data for simplicity.
 * Check {@link no.nordicsemi.android.ble.common.callback.hr.HeartRateMeasurementDataCallback}
 * for full implementation.
 *
 * TODO Modify the content to parse your data.
 */
@SuppressWarnings("ConstantConditions")
public abstract class TemplateDataCallback implements ProfileDataCallback, TemplateCharacteristicCallback {

	@Override
	public void onDataReceived(@NonNull final BluetoothDevice device, @NonNull final Data data) {

		final int value = data.getIntValue(Data.FORMAT_UINT16, 0);
		final int value1 = data.getIntValue(Data.FORMAT_UINT16, 2);
		final int value2 = data.getIntValue(Data.FORMAT_UINT16, 4);
		final int value3 = data.getIntValue(Data.FORMAT_UINT16, 6);
		final int value4 = data.getIntValue(Data.FORMAT_UINT16, 8);
		final int value5 = data.getIntValue(Data.FORMAT_UINT16, 10);

		// Report the parsed value(s)
		onSampleValueReceived(device, value, value1, value2, value3, value4, value5);

	}
	
}
