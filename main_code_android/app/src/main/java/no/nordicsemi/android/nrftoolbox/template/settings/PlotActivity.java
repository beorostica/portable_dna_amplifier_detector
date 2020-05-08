package no.nordicsemi.android.nrftoolbox.template.settings;

import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.Viewport;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

import no.nordicsemi.android.nrftoolbox.R;

import static no.nordicsemi.android.nrftoolbox.template.settings.FilesActivity.EXTRA_DATA_FILENAME;

public class PlotActivity extends AppCompatActivity {

    // Some definitions for graphView:
    private static final int MAX_DATA_POINTS = 100;
    private LineGraphSeries<DataPoint> mSerieDetectionSystem0 = new LineGraphSeries<>();
    private LineGraphSeries<DataPoint> mSerieDetectionSystem1 = new LineGraphSeries<>();
    private LineGraphSeries<DataPoint> mSerieDetectionSystem2 = new LineGraphSeries<>();
    private LineGraphSeries<DataPoint> mSerieDetectionSystem3 = new LineGraphSeries<>();

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_plot);

        // Get the file name selected from the parent activity:
        Intent intent = getIntent();
        String fileName = intent.getStringExtra(EXTRA_DATA_FILENAME);
        Toast.makeText(this, fileName, Toast.LENGTH_SHORT).show();

        // Read the saved file and append data to the graph serie:
        File mBaseFolder = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM).getAbsolutePath() + "/DNAamplifier");
        File mFile = new File(mBaseFolder + "/" + fileName);
        try {
            BufferedReader br = new BufferedReader(new FileReader(mFile));
            int counterLine = 0;
            String stringLine;
            while ((stringLine = br.readLine()) != null) {

                if (counterLine == 0) {
                    Log.v("Plot", "Column Line");
                } else {
                    String[] arrayLine = stringLine.split(",");
                    for (int i = 0; i < arrayLine.length; i++) {
                        int index = Integer.parseInt(arrayLine[1]);
                        int time = Integer.parseInt(arrayLine[2]);
                        int lightAfter = Integer.parseInt(arrayLine[6]);
                        switch (index) {
                            case 0:
                                mSerieDetectionSystem0.appendData(new DataPoint((double)time,(double)lightAfter),true, MAX_DATA_POINTS);
                                break;
                            case 1:
                                mSerieDetectionSystem1.appendData(new DataPoint((double)time,(double)lightAfter),true, MAX_DATA_POINTS);
                                break;
                            case 2:
                                mSerieDetectionSystem2.appendData(new DataPoint((double)time,(double)lightAfter),true, MAX_DATA_POINTS);
                                break;
                            case 3:
                                mSerieDetectionSystem3.appendData(new DataPoint((double)time,(double)lightAfter),true, MAX_DATA_POINTS);
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
        GraphView mGraphViewDetectionSystem = (GraphView) findViewById(R.id.graphViewDetectionSystem);
        //mGraphView.getViewport().setXAxisBoundsManual(true);
        //mGraphView.getViewport().setMaxX(MAX_DATA_POINTS);
        mSerieDetectionSystem0.setColor(Color.rgb(255,0,0));
        mSerieDetectionSystem1.setColor(Color.rgb(0,255,0));
        mSerieDetectionSystem2.setColor(Color.rgb(0,0,255));
        mSerieDetectionSystem3.setColor(Color.rgb(255,255,0));
        mGraphViewDetectionSystem.addSeries(mSerieDetectionSystem0);
        mGraphViewDetectionSystem.addSeries(mSerieDetectionSystem1);
        mGraphViewDetectionSystem.addSeries(mSerieDetectionSystem2);
        mGraphViewDetectionSystem.addSeries(mSerieDetectionSystem3);

    }

}
