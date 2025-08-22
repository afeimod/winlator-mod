package com.winlator.contentdialog;

import android.content.Context;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import com.winlator.R;
import com.winlator.core.GeneralComponents;
import com.winlator.widget.SimplePianoKeyboard;
import com.winlator.winhandler.MIDIHandler;

public class SoundFontTestDialog extends ContentDialog {
    public SoundFontTestDialog(Context context, String soundfont) {
        super(context, R.layout.soundfont_test_dialog);
        setIcon(R.drawable.icon_piano);
        setTitle(soundfont);

        String soundfontPath = GeneralComponents.getDefinitivePath(GeneralComponents.Type.SOUNDFONT, context, soundfont);

        final MIDIHandler midiHandler = new MIDIHandler(null);
        midiHandler.init();
        midiHandler.loadSoundFont(soundfontPath);

        int[] channel = {0};
        final Spinner sInstrument = findViewById(R.id.SInstrument);
        Spinner sChannel = findViewById(R.id.SChannel);
        sChannel.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int position, long id) {
                channel[0] = position == 1 ? 9 : 0;
                midiHandler.programChange(channel[0], sInstrument.getSelectedItemPosition());
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {}
        });

        sInstrument.setAdapter(new ArrayAdapter<>(context, android.R.layout.simple_spinner_dropdown_item, MIDIHandler.getInstrumentNames()));
        sInstrument.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int position, long id) {
                midiHandler.programChange(0, position);
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {}
        });

        SimplePianoKeyboard pianoKeyboard = findViewById(R.id.SimplePianoKeyboard);
        pianoKeyboard.setOnKeyListener(new SimplePianoKeyboard.OnKeyListener() {
            @Override
            public void onKeyDown(int index) {
                midiHandler.noteOn(channel[0], index + 60, 100);
            }

            @Override
            public void onKeyUp(int index) {
                midiHandler.noteOff(channel[0], index + 60);
            }
        });

        findViewById(R.id.LLBottomBar).setVisibility(View.GONE);
    }
}
