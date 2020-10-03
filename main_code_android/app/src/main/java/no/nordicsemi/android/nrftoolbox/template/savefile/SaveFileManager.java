package no.nordicsemi.android.nrftoolbox.template.savefile;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Environment;
import android.util.Log;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Calendar;
import java.util.Locale;


public class SaveFileManager {

    private Context mContext;

    FileWriter mFileWriters[] = new FileWriter[3];
    private long mTimeInitialMillis;
    private double mTimeSeconds;


    public SaveFileManager(Context context) {

        mContext = context;

        //Request user permission:
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (!(ContextCompat.checkSelfPermission(mContext, android.Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED)) {
                ActivityCompat.requestPermissions((Activity) mContext, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
            }
        }

    }

    public void createFiles(int[] dataStatArray){
        if(!areAllFileWriters()) {
            try {

                // Get the base folder to save the file:
                File mBaseFolder;

                if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
                    mBaseFolder = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM).getAbsolutePath() + "/DNAamplifier");
                } else {
                    mBaseFolder = mContext.getFilesDir();
                }

                // Create the directory if necessary:
                if (!mBaseFolder.exists()) {
                    mBaseFolder.mkdirs();
                }

                // Create the file names from the STAT characteristic:
                String fileNameBase = 20 + String.format("%02d", dataStatArray[3])  +
                        String.format("%02d", dataStatArray[4]) +
                        String.format("%02d", dataStatArray[5]) + "_" +
                        String.format("%02d", dataStatArray[6]) +
                        String.format("%02d", dataStatArray[7]) +
                        String.format("%02d", dataStatArray[8]);
                File[] mFiles = new File[mFileWriters.length];
                mFiles[0] = new File(mBaseFolder + "/" + fileNameBase + "_measure.txt");
                mFiles[1] = new File(mBaseFolder + "/" + fileNameBase + "_control.txt");
                mFiles[2] = new File(mBaseFolder + "/" + fileNameBase + "_battery.txt");

                // Create the array of file writer objects:
                for (int i = 0; i < mFileWriters.length; i++) {
                    if (!mFiles[i].isFile()) {
                        // Write the first line (must be the name columns of the data to be written):
                        mFileWriters[i] = new FileWriter(mFiles[i], true);
                        switch (i) {
                            case 0:
                                mFileWriters[i].write("timeWrite,index,time,mosfetBefore,mosfetAfter,lightBefore,lightAfter\r\n");
                                break;
                            case 1:
                                mFileWriters[i].write("timeWrite,time,refAdc,yAdc,uPwm,refAdcHotlid,yAdcHotlid,uPwmHotlid,yAdcTamb\r\n");
                                break;
                            case 2:
                                mFileWriters[i].write("timeWrite,time,soc,capacityRemain,capacityFull,soh,voltage,current,power\r\n");
                                break;
                        }
                    } else {
                        mFileWriters[i] = new FileWriter(mFiles[i], true);
                    }
                }

                // Get the initial time
                mTimeInitialMillis = Calendar.getInstance().getTimeInMillis();
                mTimeSeconds = (Calendar.getInstance().getTimeInMillis() - mTimeInitialMillis) / 1000.0;

            } catch (IOException ioe) {
                ioe.printStackTrace();
            }
        }
    }


    public void writeLine(int[] dataArray, int index){
        if(mFileWriters[index] != null) {
            try {
                mTimeSeconds = (Calendar.getInstance().getTimeInMillis() - mTimeInitialMillis) / 1000.0;
                mFileWriters[index].write(String.format(Locale.ENGLISH,"%.1f", mTimeSeconds));
                mFileWriters[index].write(",");
                for(int i = 0; i < (dataArray.length-1); i++){
                    mFileWriters[index].write(String.valueOf(dataArray[i]));
                    mFileWriters[index].write(",");
                }
                mFileWriters[index].write(String.valueOf(dataArray[dataArray.length-1]));
                mFileWriters[index].write("\r\n");
            } catch (IOException ioe){
                ioe.printStackTrace();
            }
        }
    }


    public void closeFiles(){
        if(areAllFileWriters()) {
            try {
                for(int i = 0; i < mFileWriters.length; i++){
                    mFileWriters[i].flush();
                    mFileWriters[i].close();
                    mFileWriters[i] = null;
                }
            } catch (IOException ioe) {
                ioe.printStackTrace();
            }
        }
    }

    private boolean areAllFileWriters() {
        boolean areFileWriters = true;
        for (int i = 0; i < mFileWriters.length; i++) {
            if (mFileWriters[i] == null) {
                areFileWriters = false;
                break;
            }
        }
        return areFileWriters;
    }

}
