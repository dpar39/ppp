git init
git add .
git commit -m "Deploy From git"
git remote add azure https://dpardina@passport-photo.scm.azurewebsites.net:443/passport-photo.git
git pull --rebase azure master
git push azure master