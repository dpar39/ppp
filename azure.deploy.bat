

cd %~dp0\..

git clone https://dpardinas@passport-photo.scm.azurewebsites.net:443/passport-photo.git azure

xcopy %~dp0\webapp azure /y /d /i

cd azure

git add .

git commit -m "Deploy"

git push origin master

cd %~dp0