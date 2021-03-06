package no.nordicsemi.android.nrftoolbox.template.callback;

import android.bluetooth.BluetoothDevice;

import androidx.annotation.NonNull;

/**
 * This class defines your characteristic API.
 * In this example (that is the HRM characteristic, which the template is based on), is notifying
 * with a value (Heart Rate). The single method just returns the value and ignores other
 * optional data from Heart Rate Measurement characteristic for simplicity.
 */
public interface SensCharacteristicCallback {

	/**
	 * Called when a value is received.
	 *
	 * @param device a device from which the value was obtained.
	 * @param dataArray the new array value.
	 */
	void onCharacteristicSensUpdate(@NonNull final BluetoothDevice device, int[] dataArray);

}
