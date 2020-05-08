package no.nordicsemi.android.nrftoolbox.template.settings;

import android.content.Intent;
import android.os.Bundle;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.Viewport;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;

import no.nordicsemi.android.nrftoolbox.R;

import static no.nordicsemi.android.nrftoolbox.template.settings.FilesActivity.EXTRA_DATA_FILENAME;

public class PlotActivity extends AppCompatActivity {

    // Some definitions for graphView:
    private static final int MAX_DATA_POINTS = 100;
    private LineGraphSeries<DataPoint> mSerie = new LineGraphSeries<>();

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_plot);

        // Get the file name selected from the parent activity:
        Intent intent = getIntent();
        String fileName = intent.getStringExtra(EXTRA_DATA_FILENAME);
        Toast.makeText(this, fileName, Toast.LENGTH_SHORT).show();

        // Setup the graphView plot:
        GraphView mGraphView = (GraphView) findViewById(R.id.graphViewDetectionSystem);
        mGraphView.getViewport().setXAxisBoundsManual(true);
        mGraphView.getViewport().setMaxX(MAX_DATA_POINTS);
        for(int i = 0; i < MAX_DATA_POINTS; i++){
            mSerie.appendData(new DataPoint((double)i,(double)(i*i)),true, MAX_DATA_POINTS);
        }
        mGraphView.addSeries(mSerie);

    }

}
