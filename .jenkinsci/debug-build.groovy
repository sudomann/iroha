#!/usr/bin/env groovy

def doDebugBuild(coverageEnabled=false) {
  def dPullOrBuild = load ".jenkinsci/docker-pull-or-build.groovy"
  def manifest = load ".jenkinsci/docker-manifest.groovy"
  def pCommit = load ".jenkinsci/previous-commit.groovy"
  def parallelism = params.PARALLELISM
  def sanitizeEnabled = params.sanitize
  def fuzzingEnabled = params.fuzzing
  def platform = sh(script: 'uname -m', returnStdout: true).trim()
  def previousCommit = pCommit.previousCommitOrCurrent()
  // params are always null unless job is started
  // this is the case for the FIRST build only.
  // So just set this to same value as default.
  // This is a known bug. See https://issues.jenkins-ci.org/browse/JENKINS-41929


  sh("python analyze.py result.txt")
  plot csvFileName: 'plot-3d136de2-a268-4abc-80a1-9f31db39b92d.csv', 
    csvSeries: [
      [displayTableFlag: true, exclusionValues: '', file: 'result.csv', inclusionFlag: 'OFF', url: '']
    ], 
    group: 'iroha_build_time_graph', 
    numBuilds: '3', 
    style: 'line', 
    width: 4000, 
    width: 3000,
    title: 'Build time',
    useDescr: true,
    yaxis: 'Time, sec'
}

return this
