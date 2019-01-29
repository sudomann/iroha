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


  sh("python .jenkinsci/helpers/analyzeBuildTime.py buildTimeResult.txt")
  zip archive: true, dir: '', glob: 'buildTimeResult.csv', zipFile: 'buildTimeMeasurement.zip'
  archiveArtifacts artifacts: 'buildTimeMeasurement.zip'

  }
//   println plots

return this
