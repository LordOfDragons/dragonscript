import os
import re
import shutil
import datetime
from github import Github
from github.GitRelease import GitRelease


def run_command(cmdline: str) -> None:
    retval = os.system(cmdline)
    if retval != 0:
        raise Exception("Run command failed: rc={} cmdline='{}'".format(retval, cmdline))

class Release:
    """GitHub release wrapper."""
    reVersion = re.compile(r"^v(?P<major>[0-9]+)\.(?P<minor>[0-9]+)(\.(?P<patch>[0-9]+))?$")
    
    def __init__(self: 'Release', release: GitRelease) -> None:
        self._tag_name = release.tag_name
        self._publish_time = release.published_at
        match = Release.reVersion.match(release.tag_name).groupdict()
        self._major = match['major']
        self._minor = match['minor']
        self._patch = match['patch']
        self._version = '{}.{}.{}'.format(self._major, self._minor, self._patch)\
            if self._patch else '{}.{}'.format(self._major, self._minor)

    @property
    def tag_name(self: 'Release') -> str:
        return self._tag_name

    @property
    def major(self: 'Release') -> str:
        return self._major

    @property
    def minor(self: 'Release') -> str:
        return self._minor

    @property
    def patch(self: 'Release') -> str:
        return self._patch

    @property
    def publish_time(self: 'Release') -> datetime:
        return self._publish_time

    @property
    def version(self: 'Release') -> str:
        return self._version

    def same_minor(self: 'Release', other: 'Release') -> bool:
        return other._major == self._major and other._minor == self._minor


class DocAppBuilder:
    """Build Doc API."""
    reProjNum = re.compile(r"PROJECT_NUMBER += .+")

    def __init__(self: 'DocAppBuilder',
                 path_base: str,
                 path_artifact: str,
                 path_datafile: str,
                 path_doxyfile: str = "Doxyfile",
                 path_src: str = "src",
                 path_doc: str = "doc/html") -> None:
        self.path_doxyfile = os.path.join(path_base, path_doxyfile)
        self.path_doxyfile_in = self.path_doxyfile + '.in'
        self.path_src = os.path.join(path_base, path_src)
        self.path_doc = os.path.join(path_base, path_doc)
        self.path_artifact = path_artifact
        self.path_datafile = path_datafile

    def build(self: 'DocAppBuilder', version: str) -> None:
        shutil.rmtree(self.path_doc, True)
        self._update_doxyfile(version)
        self._build_doxyfile()
        self._copy_apidoc(version)

    def _update_doxyfile(self: 'DocAppBuilder', version: str) -> None:
        if os.path.exists(self.path_doxyfile_in):
            with open(self.path_doxyfile_in, 'r') as f:
                content = f.read()
        else:
            with open(self.path_doxyfile, 'r') as f:
                content = f.read()
        content = DocAppBuilder.reProjNum.sub(
            "PROJECT_NUMBER = {}".format(version), content)
        with open(self.path_doxyfile, 'w') as f:
            f.write(content)

    def _build_doxyfile(self: 'DocAppBuilder') -> None:
        os.chdir(self.path_src)
        run_command("doxygen {}".format(os.path.relpath(
            self.path_doxyfile, self.path_src)))

    def _copy_apidoc(self: 'DocAppBuilder', version: str) -> None:
        dest: str = os.path.join(self.path_artifact, version)
        shutil.rmtree(dest, True)

        directory = os.path.dirname(dest)
        if not os.path.exists(directory):
            os.makedirs(directory)

        shutil.copytree(self.path_doc, dest)

    def write_data_file(self: 'DocAppBuilder', versions: list[Release]) -> None:
        directory = os.path.dirname(self.path_datafile)
        if not os.path.exists(directory):
            os.makedirs(directory)

        with open(self.path_datafile, 'w') as f:
            for each in versions:
                f.write("- version: '{}'\n".format(each.version))

def find_repo_releases(repo_name: str) -> list[Release]:
    """Find all released of repository.

    Sorts releases by time from newest to oldest.
    Skips nightly release. Keeps only the latest
    minor release of releases with same minor.
    """
    with Github() as gh:
        repo = gh.get_repo(repo_name)

        releases: list[Release] = []

        for each in repo.get_releases():
            if each.tag_name == "nightly":
                continue
            try:
                releases.append(Release(each))
            except Exception:
                continue

        releases.sort(key=lambda x: x.publish_time)
        releases.reverse()
        temp_list = releases
        releases = []

        for each in temp_list:
            if not any(x for x in releases if each.same_minor(x)):
                releases.append(each)
    return releases

def build_apidoc(releases: list[Release]) -> None:
    curdir = os.getcwd()

    pathApiDoc = os.path.join(curdir, "artifacts/apidoc")
    pathDataDir = os.path.join(curdir, "_data/apidoc")
    pathBaseSrc = os.path.join(curdir, "_repository/dragonscript")

    builders: list[DocAppBuilder] = []

    builders.append(DocAppBuilder(
        path_base=pathBaseSrc,
        path_artifact=os.path.join(pathApiDoc, "dragonscript"),
        path_datafile=os.path.join(pathDataDir, "dragonscript.yml"),
        path_doxyfile="doxyfile_script",
        path_src=".",
        path_doc="doc/script/html"))

    for each in builders:
        shutil.rmtree(each.path_artifact, True)

    if not os.path.exists("_repository/dragonscript"):
        if not os.path.exists("_repository"):
            os.makedirs("_repository")
        run_command(' '.join([
            "git",
            "clone",
            "https://github.com/LordOfDragons/dragonscript.git",
            "_repository/dragonscript"]))

    os.chdir("_repository/dragonscript")

    for release in releases:
        run_command("git checkout -f '{}'".format(release.tag_name))
        run_command("git reset --hard")
        run_command("git clean -x -f")

        for builder in builders:
            builder.build(release.version)

    os.chdir(curdir)
    for each in builders:
        each.write_data_file(releases)

releases = find_repo_releases("LordOfDragons/dragonscript")
limit = 3 # 5
build_apidoc(releases[:limit])
