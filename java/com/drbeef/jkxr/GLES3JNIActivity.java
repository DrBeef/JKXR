
package com.drbeef.jkxr;


import static android.system.Os.setenv;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Build;
import android.os.Bundle;
import android.os.RemoteException;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.util.Pair;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;

import com.drbeef.externalhapticsservice.HapticServiceClient;
import com.drbeef.externalhapticsservice.HapticsConstants;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Locale;
import java.util.Vector;

@SuppressLint("SdCardPath") public class GLES3JNIActivity extends Activity implements SurfaceHolder.Callback
{
	private static String game = "";
	private static String manufacturer = "";

	private boolean hapticsEnabled = false;

	// Load the gles3jni library right away to make sure JNI_OnLoad() gets called as the very first thing.
	static
	{
		game = "jo";

		BufferedReader br;
		try {
			br = new BufferedReader(new FileReader("/sdcard/JKXR/commandline.txt"));
			String s;
			StringBuilder sb = new StringBuilder(0);
			while ((s = br.readLine()) != null)
				sb.append(s + " ");
			br.close();

			if (sb.toString().contains("ja"))
			{
				game = "ja";
			}
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		manufacturer = Build.MANUFACTURER.toLowerCase(Locale.ROOT);
		if (manufacturer.contains("oculus")) // rename oculus to meta as this will probably happen in the future anyway
		{
			manufacturer = "meta";
		}

		try
		{
			//Load manufacturer specific loader
			System.loadLibrary("openxr_loader_" + manufacturer);
			setenv("OPENXR_HMD", manufacturer, true);
		} catch (Exception e)
		{}

		System.loadLibrary( "openjk_" + game );
	}

	private static final String TAG = "JKXR";
	private static final String APPLICATION = "JKXR";


	private int permissionCount = 0;
	private static final int READ_EXTERNAL_STORAGE_PERMISSION_ID = 1;
	private static final int WRITE_EXTERNAL_STORAGE_PERMISSION_ID = 2;

	String commandLineParams;

	private SurfaceView mView;
	private SurfaceHolder mSurfaceHolder;
	private long mNativeHandle;

	// Main components
	protected static GLES3JNIActivity mSingleton;

	private Vector<HapticServiceClient> externalHapticsServiceClients = new Vector<>();

	//Use a vector of pairs, it is possible a given package _could_ in the future support more than one haptic service
	//so a map here of Package -> Action would not work.
	private static Vector<Pair<String, String>> externalHapticsServiceDetails = new Vector<>();


	public static void initialize() {
		// The static nature of the singleton and Android quirkyness force us to initialize everything here
		// Otherwise, when exiting the app and returning to it, these variables *keep* their pre exit values
		mSingleton = null;

		//Add possible external haptic service details here
		externalHapticsServiceDetails.add(Pair.create(HapticsConstants.BHAPTICS_PACKAGE, HapticsConstants.BHAPTICS_ACTION_FILTER));
		externalHapticsServiceDetails.add(Pair.create(HapticsConstants.FORCETUBE_PACKAGE, HapticsConstants.FORCETUBE_ACTION_FILTER));
	}

	public void shutdown() {
		System.exit(0);
	}

	@Override protected void onCreate( Bundle icicle )
	{
		Log.v( TAG, "----------------------------------------------------------------" );
		Log.v( TAG, "GLES3JNIActivity::onCreate()" );
		super.onCreate( icicle );

		GLES3JNIActivity.initialize();

		// So we can call stuff from static callbacks
		mSingleton = this;


		mView = new SurfaceView( this );
		setContentView( mView );
		mView.getHolder().addCallback( this );

		// Force the screen to stay on, rather than letting it dim and shut off
		// while the user is watching a movie.
		getWindow().addFlags( WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON );

		// Force screen brightness to stay at maximum
		WindowManager.LayoutParams params = getWindow().getAttributes();
		params.screenBrightness = 1.0f;
		getWindow().setAttributes( params );

		checkPermissionsAndInitialize();
	}

	/** Initializes the Activity only if the permission has been granted. */
	private void checkPermissionsAndInitialize() {
		// Boilerplate for checking runtime permissions in Android.
		if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
				!= PackageManager.PERMISSION_GRANTED){
			ActivityCompat.requestPermissions(
					GLES3JNIActivity.this,
					new String[] {Manifest.permission.WRITE_EXTERNAL_STORAGE},
					WRITE_EXTERNAL_STORAGE_PERMISSION_ID);
		}
		else
		{
			permissionCount++;
		}

		if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE)
				!= PackageManager.PERMISSION_GRANTED)
		{
			ActivityCompat.requestPermissions(
					GLES3JNIActivity.this,
					new String[] {Manifest.permission.READ_EXTERNAL_STORAGE},
					READ_EXTERNAL_STORAGE_PERMISSION_ID);
		}
		else
		{
			permissionCount++;
		}

		if (permissionCount == 2) {
			// Permissions have already been granted.
			create();
		}
	}

	/** Handles the user accepting the permission. */
	@Override
	public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
		if (requestCode == READ_EXTERNAL_STORAGE_PERMISSION_ID) {
			if (results.length > 0 && results[0] == PackageManager.PERMISSION_GRANTED) {
				permissionCount++;
			}
			else
			{
				System.exit(0);
			}
		}

		if (requestCode == WRITE_EXTERNAL_STORAGE_PERMISSION_ID) {
			if (results.length > 0 && results[0] == PackageManager.PERMISSION_GRANTED) {
				permissionCount++;
			}
			else
			{
				System.exit(0);
			}
		}

		checkPermissionsAndInitialize();
	}

	public void create() {
		//Make the directories
		new File("/sdcard/JKXR/JK2/base").mkdirs();
		new File("/sdcard/JKXR/JK3/base").mkdirs();

		//Copy the command line params file
		copy_asset("/sdcard/JKXR", "commandline.txt", false);

		//Our assets
		copy_asset("/sdcard/JKXR/JK2/base", "z_vr_assets_base.pk3", true);
		copy_asset("/sdcard/JKXR/JK2/base", "z_vr_assets_jko.pk3", true);
		copy_asset("/sdcard/JKXR/JK3/base", "z_vr_assets_base.pk3", true);
		copy_asset("/sdcard/JKXR/JK3/base", "z_vr_assets_jka.pk3", true);

		//Bunch of cool mods and their credits - only copy if user wants them
		if (!new File("/sdcard/JKXR/JK2/base/no_copy").exists()) {
			copy_asset("/sdcard/JKXR/JK2/base", "packaged_mods_credits.txt", false);
			copy_asset("/sdcard/JKXR/JK2/base", "GGDynamicWeapons.pk3", false);

			//Weapon Models
			copy_asset("/sdcard/JKXR/JK2/base", "z_vr_weapons_jko_Crusty_and_Elin.pk3", true);
			copy_asset("/sdcard/JKXR/JK2/base", "assets6_vr_weapons_shaders.pk3", true);
            //Delete weapons pak and config with old name
            delete_asset(new File("/sdcard/JKXR/JK2/base/z_Crusty_and_Elin_vr_weapons.pk3"));
            delete_asset(new File("/sdcard/JKXR/JK2/base/weapons_vr_jo.cfg"));
		}

		//Bunch of cool mods and their credits - only copy if user wants them
		if (!new File("/sdcard/JKXR/JK3/base/no_copy").exists()) {
			//Weapon Models
			copy_asset("/sdcard/JKXR/JK3/base", "z_vr_weapons_jka_Crusty_and_Elin.pk3", true);
            //Delete weapons pak with old name
            delete_asset(new File("/sdcard/JKXR/JK3/base/z_Crusty_and_Elin_vr_weapons.pk3"));
            delete_asset(new File("/sdcard/JKXR/JK3/base/weapons_vr_ja.cfg"));
		}

		//Copy mods to the demo folder if demo assets exist, since the demo doesn't seem to be able to load mods from base
		if (new File("/sdcard/JKXR/JK2/jk2demo/assets0.pk3").exists() ||
			new File("/sdcard/JKXR/JK2/jk2demo/jk2demo/assets0.pk3").exists()) {

			String demoFolder = "/sdcard/JKXR/JK2/jk2demo";
			if(new File("/sdcard/JKXR/JK2/jk2demo/jk2demo/assets0.pk3").exists())
			{
				demoFolder = "/sdcard/JKXR/JK2/jk2demo/jk2demo";
			}
			//Copy the weapon adjustment config - should we force overwrite?
			copy_asset(demoFolder, "weapons_vr_jo.cfg", true);

			//Our assets
			copy_asset(demoFolder, "z_vr_assets_base.pk3", true);
			copy_asset(demoFolder, "z_vr_assets_jko.pk3", true);

			//Bunch of cool mods and their credits - only copy if user wants them
			copy_asset(demoFolder, "packaged_mods_credits.txt", false);
			copy_asset(demoFolder, "GGDynamicWeapons.pk3", false);

            //Weapon Models
            copy_asset(demoFolder, "z_vr_weapons_jko_Crusty_and_Elin.pk3", true);
            copy_asset(demoFolder, "assets6_vr_weapons_shaders.pk3", true);
            //Delete weapons pak and config with old name
            delete_asset(new File(demoFolder + "/z_Crusty_and_Elin_vr_weapons.pk3"));
            delete_asset(new File(demoFolder + "/weapons_vr_jo.cfg"));
		}

		//Read these from a file and pass through
		commandLineParams = new String("jo");

		//See if user is trying to use command line params
		if (new File("/sdcard/JKXR/commandline.txt").exists()) // should exist!
		{
			BufferedReader br;
			try {
				br = new BufferedReader(new FileReader("/sdcard/JKXR/commandline.txt"));
				String s;
				StringBuilder sb = new StringBuilder(0);
				while ((s = br.readLine()) != null)
					sb.append(s + " ");
				br.close();

				commandLineParams = new String(sb.toString());
			} catch (FileNotFoundException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		try {
			setenv("JK_LIBDIR", getApplicationInfo().nativeLibraryDir, true);
		}
		catch (Exception e)
		{

		}

		for (Pair<String, String> serviceDetail : externalHapticsServiceDetails) {
			HapticServiceClient client = new HapticServiceClient(this, (state, desc) -> {
				Log.v(APPLICATION, "ExternalHapticsService " + serviceDetail.second + ": " + desc);
			}, new Intent(serviceDetail.second)
					.setPackage(serviceDetail.first));

			client.bindService();
			externalHapticsServiceClients.add(client);
		}

		mNativeHandle = GLES3JNILib.onCreate( this, commandLineParams );
	}

	public void copy_asset_with_rename(String path, String name, String name_out, boolean force) {
		File f = new File(path + "/" + name_out);
		if (!f.exists() || force) {
			
			//Ensure we have an appropriate folder
			String fullname = path + "/" + name;
			String directory = fullname.substring(0, fullname.lastIndexOf("/"));
			new File(directory).mkdirs();
			_copy_asset(name, path + "/" + name_out);
		}
	}

	public void copy_asset(String path, String name, boolean force) {
		File f = new File(path + "/" + name);
		if (!f.exists() || force) {

			//Ensure we have an appropriate folder
			String fullname = path + "/" + name;
			String directory = fullname.substring(0, fullname.lastIndexOf("/"));
			new File(directory).mkdirs();
			_copy_asset(name, path + "/" + name);
		}
	}

	public void _copy_asset(String name_in, String name_out) {
		AssetManager assets = this.getAssets();

		try {
			InputStream in = assets.open(name_in);
			OutputStream out = new FileOutputStream(name_out);

			copy_stream(in, out);

			out.close();
			in.close();

		} catch (Exception e) {

			e.printStackTrace();
		}

	}

	public static void copy_stream(InputStream in, OutputStream out)
			throws IOException {
		byte[] buf = new byte[1024];
		while (true) {
			int count = in.read(buf);
			if (count <= 0)
				break;
			out.write(buf, 0, count);
		}
	}

	public void delete_asset(File file) {
		if (!file.exists()) {
			return;
		}
		if (file.isDirectory()) {
			for (File nestedFile : file.listFiles()) {
				delete_asset(nestedFile);
			}
		}
		file.delete();
	}

	@Override protected void onStart()
	{
		Log.v( TAG, "GLES3JNIActivity::onStart()" );
		super.onStart();

		if ( mNativeHandle != 0 )
		{
			GLES3JNILib.onStart(mNativeHandle, this);
		}
	}

	@Override protected void onResume()
	{
		Log.v( TAG, "GLES3JNIActivity::onResume()" );
		super.onResume();

		if ( mNativeHandle != 0 )
		{
			GLES3JNILib.onResume(mNativeHandle);
		}
	}

	@Override protected void onPause()
	{
		Log.v( TAG, "GLES3JNIActivity::onPause()" );
		if ( mNativeHandle != 0 )
		{
			GLES3JNILib.onPause(mNativeHandle);
		}
		super.onPause();
	}

	@Override protected void onStop()
	{
		Log.v( TAG, "GLES3JNIActivity::onStop()" );
		if ( mNativeHandle != 0 )
		{
			GLES3JNILib.onStop(mNativeHandle);
		}
		super.onStop();
	}

	@Override protected void onDestroy()
	{
		Log.v( TAG, "GLES3JNIActivity::onDestroy()" );

		if ( mSurfaceHolder != null )
		{
			GLES3JNILib.onSurfaceDestroyed( mNativeHandle );
		}

		if ( mNativeHandle != 0 )
		{
			GLES3JNILib.onDestroy(mNativeHandle);
		}

		super.onDestroy();
		// Reset everything in case the user re opens the app
		GLES3JNIActivity.initialize();
		mNativeHandle = 0;
	}

	@Override public void surfaceCreated( SurfaceHolder holder )
	{
		Log.v( TAG, "GLES3JNIActivity::surfaceCreated()" );
		if ( mNativeHandle != 0 )
		{
			GLES3JNILib.onSurfaceCreated( mNativeHandle, holder.getSurface() );
			mSurfaceHolder = holder;
		}
	}

	@Override public void surfaceChanged( SurfaceHolder holder, int format, int width, int height )
	{
		Log.v( TAG, "GLES3JNIActivity::surfaceChanged()" );
		if ( mNativeHandle != 0 )
		{
			GLES3JNILib.onSurfaceChanged( mNativeHandle, holder.getSurface() );
			mSurfaceHolder = holder;
		}
	}
	
	@Override public void surfaceDestroyed( SurfaceHolder holder )
	{
		Log.v( TAG, "GLES3JNIActivity::surfaceDestroyed()" );
		if ( mNativeHandle != 0 )
		{
			GLES3JNILib.onSurfaceDestroyed( mNativeHandle );
			mSurfaceHolder = null;
		}
	}

	public void haptic_event(String event, int position, int flags, int intensity, float angle, float yHeight)  {

		boolean areHapticsEnabled = hapticsEnabled;
		for (HapticServiceClient externalHapticsServiceClient : externalHapticsServiceClients) {

			if (externalHapticsServiceClient.hasService()) {
				try {
					//Enabled all haptics services if required
					if (!areHapticsEnabled)
					{
						externalHapticsServiceClient.getHapticsService().hapticEnable();
						hapticsEnabled = true;
						continue;
					}

					//Uses the Doom3Quest and RTCWQuest haptic patterns
					String app = "Doom3Quest";
					String eventID = event;
					if (event.contains(":"))
					{
						String[] items = event.split(":");
						app = items[0];
						eventID = items[1];
					}
					externalHapticsServiceClient.getHapticsService().hapticEvent(app, eventID, position, flags, intensity, angle, yHeight);
				}
				catch (RemoteException r)
				{
					Log.v(TAG, r.toString());
				}
			}
		}
	}

	public void haptic_updateevent(String event, int intensity, float angle) {

		for (HapticServiceClient externalHapticsServiceClient : externalHapticsServiceClients) {

			if (externalHapticsServiceClient.hasService()) {
				try {
					externalHapticsServiceClient.getHapticsService().hapticUpdateEvent(APPLICATION, event, intensity, angle);
				} catch (RemoteException r) {
					Log.v(APPLICATION, r.toString());
				}
			}
		}
	}

	public void haptic_stopevent(String event) {

		for (HapticServiceClient externalHapticsServiceClient : externalHapticsServiceClients) {

			if (externalHapticsServiceClient.hasService()) {
				try {
					externalHapticsServiceClient.getHapticsService().hapticStopEvent(APPLICATION, event);
				} catch (RemoteException r) {
					Log.v(APPLICATION, r.toString());
				}
			}
		}
	}

	public void haptic_endframe() {

		for (HapticServiceClient externalHapticsServiceClient : externalHapticsServiceClients) {

			if (externalHapticsServiceClient.hasService()) {
				try {
					externalHapticsServiceClient.getHapticsService().hapticFrameTick();
				} catch (RemoteException r) {
					Log.v(APPLICATION, r.toString());
				}
			}
		}
	}

	public void haptic_enable() {

		for (HapticServiceClient externalHapticsServiceClient : externalHapticsServiceClients) {

			if (externalHapticsServiceClient.hasService()) {
				try {
					externalHapticsServiceClient.getHapticsService().hapticEnable();
				} catch (RemoteException r) {
					Log.v(APPLICATION, r.toString());
				}
			}
		}
	}

	public void haptic_disable() {

		for (HapticServiceClient externalHapticsServiceClient : externalHapticsServiceClients) {

			if (externalHapticsServiceClient.hasService()) {
				try {
					externalHapticsServiceClient.getHapticsService().hapticDisable();
				} catch (RemoteException r) {
					Log.v(APPLICATION, r.toString());
				}
			}
		}
	}
}
