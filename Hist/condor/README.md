
To process multiple files or large datasets, submit jobs to a Condor batch system.

### 1. Create Condor Job Files

Navigate to the `condor` directory and generate the job files:

```bash
python createJobFiles.py
```

This script will generate job submission files and scripts in the `tmpSub` directory.

### 2. Submit Jobs

It's recommended to submit a few jobs first to ensure everything is working correctly. Then, submit all jobs:

```bash
cd tmpSub

# Submit all jobs
source submitAll.sh
```

### 3. Monitor Condor Jobs

Monitor the status of your Condor jobs using the following commands:

```bash
# Check the queue
condor_q

# Tail the output of a specific job
condor_tail <JobID>

# Analyze why jobs are not running
condor_q -better-analyze <JobID>
```

Replace `<JobID>` with the specific job ID from `condor_q`.

## Checking and Resubmitting Jobs

Once all jobs have completed, check for any failed jobs and resubmit them if necessary.

### 1. Check Finished Jobs

Navigate back to the `condor` directory and run the `checkFinishedJobs.py` script:

```bash
cd ..
python3 checkFinishedJobs.py
```

This script will:

- Open each output file.
- Perform checks to ensure the job ran successfully.
- Identify any failed jobs.
- Create JDL files for resubmitting failed jobs.

### 2. Resubmit Failed Jobs

If there are failed jobs, resubmit them:

```bash
cd tmpSub

# Submit the resubmission JDL file
condor_submit resubJobs.jdl
```

### 3. Repeat Checking Process

After the resubmitted jobs have completed, repeat the checking process:

```bash
cd ..
python3 checkFinishedJobs.py
```

