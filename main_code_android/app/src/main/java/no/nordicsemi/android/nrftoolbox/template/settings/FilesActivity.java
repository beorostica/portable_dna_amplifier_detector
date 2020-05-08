package no.nordicsemi.android.nrftoolbox.template.settings;

import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import androidx.appcompat.app.AppCompatActivity;

import java.io.File;
import java.util.ArrayList;

import no.nordicsemi.android.nrftoolbox.R;

public class FilesActivity extends AppCompatActivity {

    public static final String EXTRA_DATA_FILENAME = "no.nordicsemi.android.nrftoolbox.template.settings.EXTRA_DATA_FILENAME";

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_files);

        // Get the base folder where the files are saved:
        File mBaseFolder = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM).getAbsolutePath() + "/DNAamplifier");

        // Read the file names in the base folder and fill the array list of file names:
        File[] arrayFiles = mBaseFolder.listFiles();
        final ArrayList<String> arrayListFileNames = new ArrayList<String>();
        for (int i = 0; i < arrayFiles.length; i++) {
            arrayListFileNames.add(arrayFiles[i].getName());
        }

        // Setup the array adapter + list view:
        ArrayAdapter<String> itemsAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, arrayListFileNames);
        ListView listViewFiles = (ListView) findViewById(R.id.listViewFiles);
        listViewFiles.setAdapter(itemsAdapter);

        // Setup on click item listener
        listViewFiles.setOnItemClickListener(new AdapterView.OnItemClickListener(){
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                String fileName = arrayListFileNames.get(position);

                Intent intent = new Intent(FilesActivity.this, PlotActivity.class);
                intent.putExtra(EXTRA_DATA_FILENAME, fileName);
                startActivity(intent);
            }
        });


    }

}
