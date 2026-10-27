package com.basara4.android;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Button;
import android.widget.ProgressBar;
import android.content.Intent;
import android.net.Uri;
import android.os.Environment;
import android.widget.Toast;
import java.io.File;
import java.util.Arrays;
import java.util.List;

public class MainActivity extends Activity {
    
    private static final String TAG = "Basara4Android";
    private static final String GAME_ID = "CUSA01159";
    private static final String GAME_FOLDER = "/sdcard/basara4/" + GAME_ID;
    
    private ProgressBar progressBar;
    private TextView statusText;
    private boolean gameFound = false;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setupUI();
        checkGameFiles();
    }
    
    private void setupUI() {
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(32, 48, 32, 32);
        root.setGravity(android.view.Gravity.CENTER_HORIZONTAL);
        
        // Title
        TextView title = new TextView(this);
        title.setText("Sengoku Basara 4");
        title.setTextSize(32f);
        title.setTextColor(0xFFFFD700);
        title.setGravity(android.view.Gravity.CENTER);
        root.addView(title, new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT));
        
        // Subtitle
        TextView subtitle = new TextView(this);
        subtitle.setText("Sumeragi · Android Native Port");
        subtitle.setTextSize(16f);
        subtitle.setTextColor(0xFFAAAAAA);
        subtitle.setGravity(android.view.Gravity.CENTER);
        subtitle.setPadding(0, 8, 0, 24);
        root.addView(subtitle, new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT));
        
        // Divider
        View divider = new View(this);
        divider.setBackgroundColor(0xFF444444);
        LinearLayout.LayoutParams divParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, 2);
        divParams.setMargins(0, 16, 0, 16);
        root.addView(divider, divParams);
        
        // Status text
        statusText = new TextView(this);
        statusText.setText("Checking game files...");
        statusText.setTextSize(14f);
        statusText.setTextColor(0xFF888888);
        statusText.setGravity(android.view.Gravity.CENTER);
        root.addView(statusText, new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT));
        
        // Progress bar
        progressBar = new ProgressBar(this, null, android.R.attr.progressBarStyleHorizontal);
        progressBar.setProgress(0);
        progressBar.setMax(100);
        progressBar.setIndeterminate(false);
        LinearLayout.LayoutParams progressParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, 24);
        progressParams.setMargins(0, 8, 0, 16);
        root.addView(progressBar, progressParams);
        
        // Info text
        TextView info = new TextView(this);
        info.setText("Requires:\n• eboot.bin\n• sce_module/\n• sce_sys/\n\nGame dump from your PS4");
        info.setTextSize(12f);
        info.setTextColor(0xFF666666);
        info.setGravity(android.view.Gravity.CENTER);
        info.setPadding(0, 0, 0, 16);
        root.addView(info, new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT));
        
        // Play button
        Button playBtn = new Button(this);
        playBtn.setText("▶ PLAY");
        playBtn.setTextSize(18f);
        playBtn.setAllCaps(false);
        playBtn.setEnabled(false);
        playBtn.setBackgroundColor(0xFF8B0000);
        playBtn.setTextColor(0xFFFFFFFF);
        playBtn.setPadding(32, 16, 32, 16);
        playBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onPlayClicked();
            }
        });
        root.addView(playBtn, new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT));
        
        // Settings button
        Button settingsBtn = new Button(this);
        settingsBtn.setText("⚙ SETTINGS");
        settingsBtn.setTextSize(14f);
        settingsBtn.setAllCaps(false);
        settingsBtn.setBackgroundColor(0xFF333333);
        settingsBtn.setTextColor(0xFFAAAAAA);
        settingsBtn.setPadding(32, 12, 32, 12);
        settingsBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showSettings();
            }
        });
        root.addView(settingsBtn, new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT));
        
        // Footer
        TextView footer = new TextView(this);
        footer.setText("Native Android Port v1.0-dev\nBuilt with FEXCore + shadPS4");
        footer.setTextSize(10f);
        footer.setTextColor(0xFF555555);
        footer.setGravity(android.view.Gravity.CENTER);
        footer.setPadding(0, 24, 0, 0);
        root.addView(footer, new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT));
        
        setContentView(root);
    }
    
    private void checkGameFiles() {
        File gameDir = new File(GAME_FOLDER);
        
        if (!gameDir.exists()) {
            statusText.setText("Game folder not found.\nCreating folder...");
            gameDir.mkdirs();
        }
        
        List<String> requiredFiles = Arrays.asList(
            "eboot.bin",
            "sce_module",
            "sce_sys"
        );
        
        int found = 0;
        StringBuilder fileStatus = new StringBuilder();
        
        for (String f : requiredFiles) {
            File file = new File(gameDir, f);
            boolean exists = file.exists();
            if (exists) found++;
            fileStatus.append(f).append(": ").append(exists ? "✓" : "✗").append("\n");
        }
        
        progressBar.setProgress((found * 100) / requiredFiles.size());
        statusText.setText(fileStatus.toString().trim());
        
        if (found == requiredFiles.size()) {
            gameFound = true;
            statusText.setText("Game found!\nTap PLAY to start");
            statusText.setTextColor(0xFF00FF00);
            
            Button playBtn = (Button) findViewById(android.R.id.empty);
            if (playBtn == null) {
                // Find the play button programmatically
                View[] views = findViewsByType(this, Button.class);
                for (View v : views) {
                    if (v instanceof Button && ((Button)v).getText().toString().contains("PLAY")) {
                        ((Button)v).setEnabled(true);
                        ((Button)v).setBackgroundColor(0xFF450000);
                        break;
                    }
                }
            }
        } else {
            statusText.append("\n\nPlace game dump in:\n" + GAME_FOLDER);
        }
    }
    
    private void onPlayClicked() {
        if (!gameFound) {
            Toast.makeText(this, "Game files not found!", Toast.LENGTH_LONG).show();
            return;
        }
        
        Toast.makeText(this, "Starting game...\n(Requires native libraries)", 
            Toast.LENGTH_LONG).show();
        
        // In real implementation, would launch the native game
        // For now, show info about what's needed
        showInfoDialog();
    }
    
    private void showSettings() {
        Toast.makeText(this, "Settings panel coming soon", 0).show();
    }
    
    private void showInfoDialog() {
        android.app.AlertDialog.Builder builder = new android.app.AlertDialog.Builder(this);
        builder.setTitle("About This Build")
            .setMessage("This is a development stub APK.\n\n" +
                       "The native game requires:\n" +
                       "• FEXCore JIT (x86_64 → ARM64)\n" +
                       "• shadPS4 GPU core\n" +
                       "• Custom runtime libraries\n\n" +
                       "These must be built from source on a PC and copied to the device.")
            .setPositiveButton("OK", null)
            .show();
    }
    
    @SuppressWarnings("unchecked")
    private View[] findViewsByType(Activity activity, Class<?> type) {
        View rootView = activity.getWindow().getDecorView();
        List<View> result = new java.util.ArrayList<>();
        collectViewsByType(rootView, type, result);
        return result.toArray(new View[0]);
    }
    
    private void collectViewsByType(View view, Class<?> type, List<View> result) {
        if (type.isInstance(view)) {
            result.add(view);
        }
        if (view instanceof LinearLayout) {
            LinearLayout ll = (LinearLayout) view;
            for (int i = 0; i < ll.getChildCount(); i++) {
                collectViewsByType(ll.getChildAt(i), type, result);
            }
        }
    }
}