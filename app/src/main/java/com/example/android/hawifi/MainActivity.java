/*package com.example.android.hawifi;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }
}


*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.example.android.hawifi;

        import android.annotation.TargetApi;
        import android.net.wifi.WifiInfo;
        import android.net.wifi.WifiManager;
        import android.os.AsyncTask;
        import android.os.Build;
        import android.os.Bundle;
        import android.os.SystemClock;
        import android.support.v4.app.FragmentActivity;
        import android.util.TypedValue;
        import android.view.Menu;
        import android.view.MenuItem;
        import android.view.View;
        import android.widget.CompoundButton;
        import android.widget.EditText;
        import android.widget.RadioButton;
        import android.widget.SeekBar;
        import android.widget.Switch;
        import android.widget.ToggleButton;

        import com.example.android.common.logger.Log;
        import com.example.android.common.logger.LogFragment;
        import com.example.android.common.logger.LogWrapper;
        import com.example.android.common.logger.MessageOnlyLogFilter;

        import java.io.IOException;
        import java.io.InputStream;
        import java.io.InputStreamReader;
        import java.io.Reader;
        import java.io.UnsupportedEncodingException;
        import java.net.HttpURLConnection;
        import java.net.InetSocketAddress;
        import java.net.Proxy;
        import java.net.URL;
        import java.nio.charset.Charset;
        import java.security.NoSuchAlgorithmException;

        import javax.crypto.Mac;
        import javax.crypto.spec.SecretKeySpec;

/**
 * Sample application demonstrating how to connect to the network and fetch raw
 * HTML. It uses AsyncTask to do the fetch on a background thread. To establish
 * the network connection, it uses HttpURLConnection.
 *
 * This sample uses the logging framework to display log output in the log
 * fragment (LogFragment).
 */
public class MainActivity extends FragmentActivity {

    public static final String TAG = "Network Connect";
    private static final boolean D = true;
    private Switch mSwitch;
    private ToggleButton mTb;
    private RadioButton mRb;

    private class ardUrl {
        public String url = "";
        public boolean stat = false;
    }
    private ardUrl[] myUrls = new ardUrl[5];

    // Reference to the fragment showing events, so we can clear it with a button
    // as necessary.
    private LogFragment mLogFragment;

    @TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Initialize text fragment that displays intro text.
        /*SimpleTextFragment introFragment = (SimpleTextFragment)
                    getSupportFragmentManager().findFragmentById(R.id.intro_fragment);
        introFragment.setText(R.string.welcome_message);
        introFragment.getTextView().setTextSize(TypedValue.COMPLEX_UNIT_DIP, 16.0f);*/

        // Initialize the logging framework.
        initializeLogging();

        // Initialize the send buttons with a listener that for click events
        mTb = (ToggleButton) findViewById(R.id.tb_gl);
        mTb.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    throwCmd("gln");
                } else {
                    throwCmd("glf");
                }
            }
        });

        mTb = (ToggleButton) findViewById(R.id.tb_dl);
        mTb.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    throwCmd("dln");
                } else {
                    throwCmd("dlf");
                }
            }
        });

        mTb = (ToggleButton) findViewById(R.id.tb_gu);
        mTb.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    throwCmd("gun");
                } else {
                    throwCmd("guf");
                }
            }
        });
        mTb = (ToggleButton) findViewById(R.id.tb_gd);
        mTb.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    throwCmd("gdn");
                } else {
                    throwCmd("gdf");
                }
            }
        });

        mTb = (ToggleButton) findViewById(R.id.tb_du);
        mTb.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    throwCmd("dun");
                } else {
                    throwCmd("duf");
                }
            }
        });
        mTb = (ToggleButton) findViewById(R.id.tb_dd);
        mTb.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    throwCmd("ddn");
                } else {
                    throwCmd("ddf");
                }
            }
        });

        mTb = (ToggleButton) findViewById(R.id.tb_pc);
        mTb.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    throwCmd("pcn");
                } else {
                    throwCmd("pcf");
                }
            }
        });
        mTb = (ToggleButton) findViewById(R.id.tb_tv);
        mTb.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    throwCmd("tvn");
                } else {
                    throwCmd("tvf");
                }
            }
        });

        mTb = (ToggleButton) findViewById(R.id.tb_pl);
        mTb.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    throwCmd("pln");
                } else {
                    throwCmd("plf");
                }
            }
        });
        mTb = (ToggleButton) findViewById(R.id.tb_pk);
        mTb.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    throwCmd("pkn");
                } else {
                    throwCmd("pkf");
                }
            }
        });

        // piotr
        findViewById(R.id.radioButton0).setEnabled(false);
        findViewById(R.id.radioButton1).setEnabled(false);
        findViewById(R.id.radioButton2).setEnabled(false);
        findViewById(R.id.radioButton3).setEnabled(false);
        findViewById(R.id.radioButton4).setEnabled(false);

        myUrls = new ardUrl[5];
        long nonce = myTime();
        String result = "";
        for (int i = 0; i < 5; i++) {
            myUrls[i] = new ardUrl();
            myUrls[i].stat = false;
        }
        myUrls[0].url = "http://piotrlech.ddns.net:60000/";
        myUrls[1].url = "http://piotrlech.ddns.net:58052/";
        myUrls[2].url = "http://192.168.1.25:84/";
        myUrls[3].url = "http://192.168.1.42:83/";
        myUrls[4].url = "http://192.168.2.42:83/";

        chkArduino asyncTask0, asyncTask1, asyncTask2, asyncTask3, asyncTask4;
        WifiManager wifiManager = (WifiManager) getSystemService(WIFI_SERVICE);
        WifiInfo wifiInfo = wifiManager.getConnectionInfo();
        String ap = wifiInfo.getSSID();
        ap = ap.replace("\"", "");
        if (ap.equals("FunBox-1EFF") || ap.equals("U4")) {
            //for (int i = 2; i < 4; i++) {
            //    new chkArduino().execute(myUrls[i].url + "stm" + "/" + nonce + "/" + result);
            //}
            asyncTask2 = new chkArduino();
            //asyncTask2.execute(myUrls[2].url + "stm" + "/" + nonce + "/" + result);
            asyncTask2.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, myUrls[2].url + "stm" + "/" + nonce + "/" + result);
            asyncTask3 = new chkArduino();
            //asyncTask3.execute(myUrls[3].url + "stm" + "/" + nonce + "/" + result);
            asyncTask3.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, myUrls[3].url + "stm" + "/" + nonce + "/" + result);
            //StartAsyncTaskInParallel(asyncTask3);
        }
        if (ap.equals("PENTAGRAM")) {
            //for (int i = 4; i < 5; i++) {
            //    new chkArduino().execute(myUrls[i].url + "stm" + "/" + nonce + "/" + result);
            //}
            asyncTask4 = new chkArduino();
            asyncTask4.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, myUrls[4].url + "stm" + "/" + nonce + "/" + result);
        }
        //for (int i = 0; i < 2; i++) {
        //    new chkArduino().execute(myUrls[i].url + "stm" + "/" + nonce + "/" + result);
        //}
        asyncTask0 = new chkArduino();
        asyncTask0.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, myUrls[0].url + "stm" + "/" + nonce + "/" + result);
        asyncTask1 = new chkArduino();
        asyncTask1.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, myUrls[1].url + "stm" + "/" + nonce + "/" + result);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    public long myTime() {
        return (System.currentTimeMillis() / 1000L - 1440409855L);
    }

    public boolean throwCmd(String command) {
        final Charset asciiCs = Charset.forName("US-ASCII");
        long nonce = myTime();
        String HMAC_PASS = "password";
        String HMAC_KEY  = "key";
        //String beforeHmac = "The quick brown fox jumps over the lazy dog";
        String beforeHmac = "/" + HMAC_PASS + "/" + command + "/" + nonce + "/";
        String result = "";
        try {
            final Mac sha256_HMAC = Mac.getInstance("HmacSHA256");
            final SecretKeySpec secret_key = new javax.crypto.spec.SecretKeySpec(asciiCs.encode(HMAC_KEY).array(), "HmacSHA256");
            sha256_HMAC.init(secret_key);
            final byte[] mac_data = sha256_HMAC.doFinal(asciiCs.encode(beforeHmac).array());
            for (final byte element : mac_data) {
                result += Integer.toString((element & 0xff) + 0x100, 16).substring(1);
            }
        } catch(Exception e) {
            if(D) Log.e(TAG, "Crypto Exception");
        }

        DownloadTask dt;
        String url = null;
        if (((RadioButton) findViewById(R.id.radioButton0)).isChecked())
            url = myUrls[0].url;
        else if (((RadioButton) findViewById(R.id.radioButton1)).isChecked())
            url = myUrls[1].url;
        else if (((RadioButton) findViewById(R.id.radioButton2)).isChecked())
            url = myUrls[2].url;
        else if (((RadioButton) findViewById(R.id.radioButton3)).isChecked())
            url = myUrls[3].url;
        else if (((RadioButton) findViewById(R.id.radioButton4)).isChecked())
            url = myUrls[4].url;
        if (url != null) {
            //new DownloadTask().execute(url + command + "/" + nonce + "/" + result);
            dt = new DownloadTask();
            dt.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, url + command + "/" + nonce + "/" + result);
        }
        Log.e(TAG, url + command);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            // When the user clicks FETCH, fetch the first 500 characters of
            case R.id.fetch_action:
                EditText tvButton = (EditText) findViewById(R.id.editText);
                String sTime = tvButton.getText().toString();
                throwCmd(sTime);
                return true;
            // Clear the log view fragment.
            case R.id.clear_action:
                mLogFragment.getLogView().setText("");
                return true;
        }
        return false;
    }

    private class chkArduino extends AsyncTask<String, Void, String> {
        private String switchString;

        @Override
        protected String doInBackground(String... urls) {
            try {
                switchString = urls[0];
                Log.i(TAG, "req:" + switchString);
                return loadFromNetwork(urls[0]);
            } catch (IOException e) {
                return e.getMessage();
            }
        }

        @Override
        protected void onPostExecute(String result) {
            if (result != null) {
                if (result.substring(0, 3).equals("Hi ") || result.substring(0, 3).equals("cmd")) {
                    Log.i(TAG, "chk:" + switchString);
                    if (myUrls[0].url.equals(switchString.substring(0, myUrls[0].url.length()))) {
                        findViewById(R.id.radioButton0).setEnabled(true);
                    } else if (myUrls[1].url.equals(switchString.substring(0, myUrls[1].url.length()))) {
                        findViewById(R.id.radioButton1).setEnabled(true);
                    } else if (myUrls[2].url.equals(switchString.substring(0, myUrls[2].url.length()))) {
                        findViewById(R.id.radioButton2).setEnabled(true);
                    } else if (myUrls[3].url.equals(switchString.substring(0, myUrls[3].url.length()))) {
                        findViewById(R.id.radioButton3).setEnabled(true);
                    } else if (myUrls[4].url.equals(switchString.substring(0, myUrls[4].url.length()))) {
                        findViewById(R.id.radioButton4).setEnabled(true);
                    } else {
                        Log.i(TAG, "222:" + myUrls[2].url + "|" + switchString.substring(0, myUrls[2].url.length()));
                        //Log.i(TAG, "333:" + myUrls[3].url + "|" + switchString.substring(0, myUrls[3].url.length()));
                    }
                }
                else {
                    Log.i(TAG, "sub:" + result.substring(0, 3));
                }
                if (findViewById(R.id.radioButton4).isEnabled())
                    ((RadioButton)findViewById(R.id.radioButton4)).setChecked(true);
                else if (findViewById(R.id.radioButton2).isEnabled())
                    ((RadioButton)findViewById(R.id.radioButton2)).setChecked(true);
                else if (findViewById(R.id.radioButton3).isEnabled())
                    ((RadioButton)findViewById(R.id.radioButton3)).setChecked(true);
                else if (findViewById(R.id.radioButton0).isEnabled())
                    ((RadioButton)findViewById(R.id.radioButton0)).setChecked(true);
                else if (findViewById(R.id.radioButton1).isEnabled())
                    ((RadioButton)findViewById(R.id.radioButton1)).setChecked(true);
            }
            Log.i(TAG, "ans:" + result);
        }
    }

    /*@TargetApi(Build.VERSION_CODES.HONEYCOMB)
    private void StartAsyncTaskInParallel(MyAsyncTask task) {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB)
            task.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        else
            task.execute();
    }*/

    /**
     * Implementation of AsyncTask, to fetch the data in the background away from
     * the UI thread.
     */
    private class DownloadTask extends AsyncTask<String, Void, String> {

        @Override
        protected String doInBackground(String... urls) {
            try {
                return loadFromNetwork(urls[0]);
            } catch (IOException e) {
                //return getString(R.string.connection_error);
                return e.getMessage();
            }
        }

        /**
         * Uses the logging framework to display the output of the fetch
         * operation in the log fragment.
         */
        @Override
        protected void onPostExecute(String result) {
            Log.i(TAG, result);
        }
    }

    /** Initiates the fetch operation. */
    private String loadFromNetwork(String urlString) throws IOException {
        InputStream stream = null;
        String str ="";

        try {
            stream = downloadUrl(urlString);
            long mStart = System.currentTimeMillis();
            while((System.currentTimeMillis() - mStart) < 100) {
                str = str + readIt(stream, 500);
                //if(D) Log.e(TAG, "i = " + i + "'" + stream + '+');
            }
            //str = readIt(stream, 500);
        } finally {
            if (stream != null) {
                stream.close();
            }
        }
        return str;
    }

    /**
     * Given a string representation of a URL, sets up a connection and gets
     * an input stream.
     * @param urlString A string representation of a URL.
     * @return An InputStream retrieved from a successful HttpURLConnection.
     * @throws java.io.IOException
     */
    private InputStream downloadUrl(String urlString) throws IOException {
        // BEGIN_INCLUDE(get_inputstream)
        URL url = new URL(urlString);
        Proxy proxy = new Proxy(Proxy.Type.HTTP, new InetSocketAddress("10.144.1.10", 8080));
        HttpURLConnection conn = (HttpURLConnection) url.openConnection(proxy);
        //HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setReadTimeout(10000 /* milliseconds */);
        conn.setConnectTimeout(15000 /* milliseconds */);
        conn.setRequestMethod("GET");
        conn.setDoInput(true);
        //conn.usingProxy();
        //conn.setAllowUserInteraction(false);
        // Start the query
        conn.connect();
        InputStream stream = conn.getInputStream();
        return stream;
        // END_INCLUDE(get_inputstream)
    }

    /** Reads an InputStream and converts it to a String.
     * @param stream InputStream containing HTML from targeted site.
     * @param len Length of string that this method returns.
     * @return String concatenated according to len parameter.
     * @throws java.io.IOException
     * @throws java.io.UnsupportedEncodingException
     */
    private String readIt(InputStream stream, int len) throws IOException, UnsupportedEncodingException {
        Reader reader = null;
        reader = new InputStreamReader(stream, "UTF-8");
        char[] buffer = new char[len];
        reader.read(buffer);
        return new String(buffer);

        /*BufferedInputStream bis = new BufferedInputStream(stream);
        ByteArrayBuffer baf = new ByteArrayBuffer(50);
        int read = 0;
        int bufSize = 512;
        byte[] bbuffer = new byte[bufSize];
        while(true){
            read = bis.read(bbuffer);
            if(read==-1){
                break;
            }
            baf.append(bbuffer, 0, read);
        }
        return new String(baf.toByteArray());*/
    }

    /** Create a chain of targets that will receive log data */
    public void initializeLogging() {

        // Using Log, front-end to the logging chain, emulates
        // android.util.log method signatures.

        // Wraps Android's native log framework
        LogWrapper logWrapper = new LogWrapper();
        Log.setLogNode(logWrapper);

        // A filter that strips out everything except the message text.
        MessageOnlyLogFilter msgFilter = new MessageOnlyLogFilter();
        logWrapper.setNext(msgFilter);

        // On screen logging via a fragment with a TextView.
        mLogFragment =
                (LogFragment) getSupportFragmentManager().findFragmentById(R.id.log_fragment);
        msgFilter.setNext(mLogFragment.getLogView());
    }
}
