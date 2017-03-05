:: ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Commits new changes on the website and deploys to azure
:: ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
pushd %~dp0\..
if not exist azure (
    git clone https://dpardinas@passport-photo.scm.azurewebsites.net:443/passport-photo.git azure    
)
xcopy %~dp0\webapp azure /y /e /exclude:node_modues+uploads
cd azure
git commit -a -m "Deploy"
git push origin master
popd