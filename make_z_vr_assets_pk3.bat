cd assets
del z_vr_assets.pk3
cd ..
powershell Compress-Archive z_vr_assets/* z_vr_assets.zip
rename z_vr_assets.zip z_vr_assets.pk3
move z_vr_assets.pk3 assets/
