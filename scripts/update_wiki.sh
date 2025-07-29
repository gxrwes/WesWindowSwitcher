echo "Updating Wiki for tag $CI_COMMIT_TAG"
# assumes you have CI_JOB_TOKEN and CI variables set

git config --global user.email "ci-bot@example.com"
git config --global user.name  "CI Bot"

# clone the wiki repo
git clone "https://gitlab-ci-token:${CI_JOB_TOKEN}@${CI_SERVER_HOST}${CI_PROJECT_PATH}.wiki.git" wiki

# copy your docs into the wiki
cp docs/*.md wiki/

cd wiki
git add .
git commit -m "Auto-update wiki for $CI_COMMIT_TAG"
git push origin masterPPP