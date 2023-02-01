cd assets
del z_vr_assets.pk3
cd ..
powershell Compress-Archive z_vr_assets/* z_vr_assets.zip
rename z_vr_assets.zip z_vr_assets.pk3
move z_vr_assets.pk3 assets/

cd assets
del z_vr_assets_jka.pk3
cd ..
powershell Compress-Archive z_vr_assets_jka/* z_vr_assets_jka.zip
rename z_vr_assets_jka.zip z_vr_assets_jka.pk3
move z_vr_assets_jka.pk3 assets/

pause
