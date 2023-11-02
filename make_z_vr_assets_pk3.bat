cd assets
del z_vr_assets_jka.pk3
cd ..
powershell Compress-Archive z_vr_assets_jka/* z_vr_assets_jka.zip
rename z_vr_assets_jka.zip z_vr_assets_jka.pk3
move z_vr_assets_jka.pk3 assets/

cd assets
del z_vr_assets_jka.pk3
cd ..
powershell Compress-Archive z_vr_assets_jka/* z_vr_assets_jka.zip
rename z_vr_assets_jka.zip z_vr_assets_jka.pk3
move z_vr_assets_jka.pk3 assets/

cd assets
del z_vr_assets_base.pk3
cd ..
powershell Compress-Archive z_vr_assets_base/* z_vr_assets_base.zip
rename z_vr_assets_base.zip z_vr_assets_base.pk3
move z_vr_assets_base.pk3 assets/

pause
