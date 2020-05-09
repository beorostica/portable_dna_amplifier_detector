package no.nordicsemi.android.nrftoolbox.template.settings;

import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

import no.nordicsemi.android.nrftoolbox.R;

import static no.nordicsemi.android.nrftoolbox.template.settings.FilesActivity.EXTRA_DATA_FILENAMEBASE;

public class PlotActivity extends AppCompatActivity {

    // Some definitions for graphView:
    private static final int MAX_DATA_POINTS = 100;

    private LineGraphSeries<DataPoint> mSerieMeasureSystem0 = new LineGraphSeries<>();
    private LineGraphSeries<DataPoint> mSerieMeasureSystem1 = new LineGraphSeries<>();
    private LineGraphSeries<DataPoint> mSerieMeasureSystem2 = new LineGraphSeries<>();
    private LineGraphSeries<DataPoint> mSerieMeasureSystem3 = new LineGraphSeries<>();

    private LineGraphSeries<DataPoint> mSerieControlSystem0 = new LineGraphSeries<>();
    private LineGraphSeries<DataPoint> mSerieControlSystem1 = new LineGraphSeries<>();

    private LineGraphSeries<DataPoint> mSerieBatterySystem0 = new LineGraphSeries<>();

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_plot);

        // Get the file name selected from the parent activity:
        Intent intent = getIntent();
        String fileNameBase = intent.getStringExtra(EXTRA_DATA_FILENAMEBASE);
        Toast.makeText(this, fileNameBase, Toast.LENGTH_SHORT).show();

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// For Detection System ///////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////

        // Read the saved file and append data to the graph serie:
        File mBaseFolder = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM).getAbsolutePath() + "/DNAamplifier");
        String fileName = fileNameBase + "_measure.txt";
        File mFile = new File(mBaseFolder + "/" + fileName);
        try {
            BufferedReader br = new BufferedReader(new FileReader(mFile));
            int counterLine = 0;
            String stringLine;
            while ((stringLine = br.readLine()) != null) {

                if (counterLine == 0) {
                    Log.v("Plot", "Column Line Measure");
                } else {
                    String[] arrayLine = stringLine.split(",");
                    for (int i = 0; i < arrayLine.length; i++) {
                        int index = Integer.parseInt(arrayLine[1]);
                        int time = Integer.parseInt(arrayLine[2]);
                        int lightAfter = Integer.parseInt(arrayLine[6]);
                        switch (index) {
                            case 0:
                                mSerieMeasureSystem0.appendData(new DataPoint((double)time,(double)lightAfter),true, MAX_DATA_POINTS);
                                break;
                            case 1:
                                mSerieMeasureSystem1.appendData(new DataPoint((double)time,(double)lightAfter),true, MAX_DATA_POINTS);
                                break;
                            case 2:
                                mSerieMeasureSystem2.appendData(new DataPoint((double)time,(double)lightAfter),true, MAX_DATA_POINTS);
                                break;
                            case 3:
                                mSerieMeasureSystem3.appendData(new DataPoint((double)time,(double)lightAfter),true, MAX_DATA_POINTS);
                                break;
                        }
                    }
                }
                counterLine += 1;

            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        // Setup the graphView plot:
        GraphView mGraphViewDetectionSystem = (GraphView) findViewById(R.id.graphViewMeasureSystem);
        //mGraphView.getViewport().setXAxisBoundsManual(true);
        //mGraphView.getViewport().setMaxX(MAX_DATA_POINTS);
        mSerieMeasureSystem0.setColor(Color.rgb(255,0,0));
        mSerieMeasureSystem1.setColor(Color.rgb(0,255,0));
        mSerieMeasureSystem2.setColor(Color.rgb(0,0,255));
        mSerieMeasureSystem3.setColor(Color.rgb(255,255,0));
        mGraphViewDetectionSystem.addSeries(mSerieMeasureSystem0);
        mGraphViewDetectionSystem.addSeries(mSerieMeasureSystem1);
        mGraphViewDetectionSystem.addSeries(mSerieMeasureSystem2);
        mGraphViewDetectionSystem.addSeries(mSerieMeasureSystem3);

        ////////////////////////////////////////////////////////////////////////////////////////////
        /// For Temp Control System ////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////

        // Read the saved file and append data to the graph serie:
        fileName = fileNameBase + "_control.txt";
        mFile = new File(mBaseFolder + "/" + fileName);
        try {
            BufferedReader br = new BufferedReader(new FileReader(mFile));
            int counterLine = 0;
            String stringLine;
            while ((stringLine = br.readLine()) != null) {

                if (counterLine == 0) {
                    Log.v("Plot", "Column Line Control");
                } else {
                    String[] arrayLine = stringLine.split(",");
                    for (int i = 0; i < arrayLine.length; i++) {
                        int time = Integer.parseInt(arrayLine[1]);
                        int refAdc = Integer.parseInt(arrayLine[2]);
                        int yAdc = Integer.parseInt(arrayLine[3]);
                        mSerieControlSystem0.appendData(new DataPoint((double)time,(double)refAdc),true, MAX_DATA_POINTS);
                        mSerieControlSystem1.appendData(new DataPoint((double)time,(double)yAdc),true, MAX_DATA_POINTS);
                    }
                }
                counterLine += 1;

            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        // Setup the graphView plot:
        GraphView mGraphViewControlSystem = (GraphView) findViewById(R.id.graphViewControlSystem);
        mSerieControlSystem0.setColor(Color.rgb(255,0,0));
        mSerieControlSystem1.setColor(Color.rgb(0,0,255));
        mGraphViewControlSystem.addSeries(mSerieControlSystem0);
        mGraphViewControlSystem.addSeries(mSerieControlSystem1);


        ////////////////////////////////////////////////////////////////////////////////////////////
        /// For Battery System /////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////

        // Read the saved file and append data to the graph serie:
        fileName = fileNameBase + "_battery.txt";
        mFile = new File(mBaseFolder + "/" + fileName);
        try {
            BufferedReader br = new BufferedReader(new FileReader(mFile));
            int counterLine = 0;
            String stringLine;
            while ((stringLine = br.readLine()) != null) {

                if (counterLine == 0) {
                    Log.v("Plot", "Column Line Battery");
                } else {
                    String[] arrayLine = stringLine.split(",");
                    for (int i = 0; i < arrayLine.length; i++) {
                        int time = Integer.parseInt(arrayLine[1]);
                        int voltage = Integer.parseInt(arrayLine[6]);
                        mSerieBatterySystem0.appendData(new DataPoint((double)time,(double)voltage),true, MAX_DATA_POINTS);
                    }
                }
                counterLine += 1;

            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        // Setup the graphView plot:
        GraphView mGraphViewBatterySystem = (GraphView) findViewById(R.id.graphViewBatterySystem);
        mSerieBatterySystem0.setColor(Color.rgb(255,0,0));
        mGraphViewBatterySystem.addSeries(mSerieBatterySystem0);

    }

}
