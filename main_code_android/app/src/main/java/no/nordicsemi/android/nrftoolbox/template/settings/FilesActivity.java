package no.nordicsemi.android.nrftoolbox.template.settings;

import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;

import no.nordicsemi.android.nrftoolbox.R;

public class FilesActivity extends AppCompatActivity {

    public static final String EXTRA_DATA_FILENAMEBASE = "no.nordicsemi.android.nrftoolbox.template.settings.EXTRA_DATA_FILENAMEBASE";

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_files);

        // For Toolbar:
        final Toolbar toolbar  = findViewById(R.id.toolbar_actionbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        // Get the base folder where the files are saved:
        File mBaseFolder = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM).getAbsolutePath() + "/DNAamplifier");

        // Read the file names in the base folder and sort them by name:
        File[] arrayFiles = mBaseFolder.listFiles();
        if (arrayFiles != null && arrayFiles.length > 1) {
            Arrays.sort(arrayFiles, new Comparator<File>() {
                @Override
                public int compare(File object1, File object2) {
                    return object2.getName().compareTo(object1.getName());
                }
            });
        }

        // Fill the array list of file names with the array of file names with the extension "_measure.txt, _control.txt, _battery.txt" removed:
        final ArrayList<String> arrayListFileNames = new ArrayList<String>();
        arrayListFileNames.add(arrayFiles[0].getName().substring(0,15));
        for (int i = 1; i < arrayFiles.length; i++) {
            if(!arrayFiles[i].getName().substring(0,15).equals(arrayFiles[i-1].getName().substring(0,15))){
                arrayListFileNames.add(arrayFiles[i].getName().substring(0,15));
            }
        }

        // Setup the array adapter + list view:
        ArrayAdapter<String> itemsAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, arrayListFileNames);
        ListView listViewFiles = (ListView) findViewById(R.id.listViewFiles);
        listViewFiles.setAdapter(itemsAdapter);

        // Setup on click item listener
        listViewFiles.setOnItemClickListener(new AdapterView.OnItemClickListener(){
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                String fileNameBase = arrayListFileNames.get(position);

                Intent intent = new Intent(FilesActivity.this, PlotActivity.class);
                intent.putExtra(EXTRA_DATA_FILENAMEBASE, fileNameBase);
                startActivity(intent);
            }
        });

    }

    // For Toolbar:
    @Override
    public boolean onOptionsItemSelected(final MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                onBackPressed();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

}
