
import React from 'react';
import Typography from '@mui/material/Typography';
import Box from '@mui/material/Box';
import Button from '@mui/material/Button';
import ButtonGroup from '@mui/material/ButtonGroup';
import IconButton from '@mui/material/IconButton';
import CircularProgress from '@mui/material/CircularProgress';
import StopCircleOutlinedIcon from '@mui/icons-material/StopCircleOutlined';
import PauseCircleOutlineOutlinedIcon from '@mui/icons-material/PauseCircleOutlineOutlined';
import NextPlanOutlinedIcon from '@mui/icons-material/NextPlanOutlined';
import NotStartedIcon from '@mui/icons-material/NotStarted';
import TimelineIcon from '@mui/icons-material/Timeline';
import { getText, } from 'src/services/string-definitions';
import { setModal, } from 'src/actions/Auth';
import { setCurrentUba, setState, setSelectedDevices} from 'src/actions/UbaDevices';
import { statusCodes, pageStateList, isStatusInPending, } from 'src/constants/unsystematic';
import {pauseRunningTest, stopRunningTest, forceStopRunningTest, resumeRunningTest, getGraphData, confirmRunningTest, nextStepRunningTest} from 'src/action-creators/TestRoutines';
import CheckCircleIcon from '@mui/icons-material/CheckCircle';

export const getActions = (row, authDispatch, ubaDevicesDispatch, testRoutinesDispatch) => {
    if (row?.status === statusCodes.RUNNING) {
        //console.log('==> RUNNING');
        if ((row?.testCurrentStep + 1) < row?.totalStagesAmount) {
            return (
                <ButtonGroup>
                    <IconButton title={getText('common.STOP')} aria-label="stop" onClick={() => handleStopTest(row, authDispatch, ubaDevicesDispatch)} >
                        <StopCircleOutlinedIcon color="error" />
                    </IconButton>

                    <IconButton title={getText('common.PAUSE')} aria-label="pause" onClick={() => handlePauseTest(row, authDispatch, ubaDevicesDispatch)} >
                        <PauseCircleOutlineOutlinedIcon color="error" />
                    </IconButton>

                    <IconButton title={getText('common.NEXT')} aria-label="next" onClick={() => handleNextTest(row, authDispatch, ubaDevicesDispatch)} >
                        <NextPlanOutlinedIcon color="error" />
                    </IconButton>

                    <IconButton title={getText('common.VIEW_GRAPH')} aria-label="graph details" onClick={() => handleGraphOpening(row, authDispatch, ubaDevicesDispatch, testRoutinesDispatch)} >
                        <TimelineIcon color="primary" />
                    </IconButton>
                </ButtonGroup>
            );
        }
        else /*if ((row?.testCurrentStep + 1) === row?.totalStagesAmount)*/ {
            return (
                <ButtonGroup>
                    <IconButton title={getText('common.STOP')} aria-label="stop" onClick={() => handleStopTest(row, authDispatch, ubaDevicesDispatch)} >
                        <StopCircleOutlinedIcon color="error" />
                    </IconButton>

                    <IconButton title={getText('common.PAUSE')} aria-label="pause" onClick={() => handlePauseTest(row, authDispatch, ubaDevicesDispatch)} >
                        <PauseCircleOutlineOutlinedIcon color="error" />
                    </IconButton>

                    <IconButton title={getText('common.VIEW_GRAPH')} aria-label="graph details" onClick={() => handleGraphOpening(row, authDispatch, ubaDevicesDispatch, testRoutinesDispatch)} >
                        <TimelineIcon color="primary" />
                    </IconButton>
                </ButtonGroup>
            );
        }
    }

    else if ((row?.status === statusCodes.NEXTSTEP) ||
              (row?.status === statusCodes.PENDING_NEXTSTEP)) {
            if ((row?.testCurrentStep + 1) < row?.totalStagesAmount) {
            return (
                <ButtonGroup>
                    <IconButton title={getText('common.STOP')} aria-label="stop" onClick={() => handleStopTest(row, authDispatch, ubaDevicesDispatch)} >
                        <StopCircleOutlinedIcon color="error" />
                    </IconButton>

                    <IconButton title={getText('common.PAUSE')} aria-label="pause" onClick={() => handlePauseTest(row, authDispatch, ubaDevicesDispatch)} >
                        <PauseCircleOutlineOutlinedIcon color="error" />
                    </IconButton>

                    <IconButton title={getText('common.NEXT')} aria-label="next" onClick={() => handleNextTest(row, authDispatch, ubaDevicesDispatch)} >
                        <NextPlanOutlinedIcon color="error" />
                    </IconButton>

                    <IconButton title={getText('common.VIEW_GRAPH')} aria-label="graph details" onClick={() => handleGraphOpening(row, authDispatch, ubaDevicesDispatch, testRoutinesDispatch)} >
                        <TimelineIcon color="primary" />
                    </IconButton>
                </ButtonGroup>
            );
        }
        else /*if ((row?.testCurrentStep + 1) === row?.totalStagesAmount)*/ {
            return (
                <ButtonGroup>
                    <IconButton title={getText('common.STOP')} aria-label="stop" onClick={() => handleStopTest(row, authDispatch, ubaDevicesDispatch)} >
                        <StopCircleOutlinedIcon color="error" />
                    </IconButton>

                    <IconButton title={getText('common.PAUSE')} aria-label="pause" onClick={() => handlePauseTest(row, authDispatch, ubaDevicesDispatch)} >
                        <PauseCircleOutlineOutlinedIcon color="error" />
                    </IconButton>

                     <IconButton title={getText('common.VIEW_GRAPH')} aria-label="graph details" onClick={() => handleGraphOpening(row, authDispatch, ubaDevicesDispatch, testRoutinesDispatch)} >
                        <TimelineIcon color="primary" />
                    </IconButton>
                </ButtonGroup>
            );
        }
    }

    else if (row?.status === statusCodes.PAUSED) {
        return (
            <ButtonGroup>
                <IconButton title={getText('common.STOP')} aria-label="stop" onClick={() => handleStopTest(row, authDispatch, ubaDevicesDispatch)} >
                    <StopCircleOutlinedIcon color="error" />
                </IconButton>

                <IconButton title={getText('common.RESUME')} aria-label="resume" onClick={() => handleResumeTest(row, authDispatch, ubaDevicesDispatch)} >
                    <NotStartedIcon color="primary" />
                </IconButton>

                <IconButton title={getText('common.VIEW_GRAPH')} aria-label="graph details" onClick={() => handleGraphOpening(row, authDispatch, ubaDevicesDispatch, testRoutinesDispatch)} >
                    <TimelineIcon color="primary" />
                </IconButton>
            </ButtonGroup>
        );
    }

    else if (
        row?.status === statusCodes.STOPPED ||
        row?.status === statusCodes.ABORTED ||
        row?.status === statusCodes.FINISHED
    ) {
        return (
            <ButtonGroup>
                <IconButton title={getText('common.CONFIRM')} aria-label="confirm" onClick={() => handleConfirmTest(row, authDispatch, ubaDevicesDispatch)} >
                    <CheckCircleIcon color="success" />
                </IconButton>
                
                <IconButton title={getText('common.VIEW_GRAPH')} aria-label="graph details" onClick={() => handleGraphOpening(row, authDispatch, ubaDevicesDispatch, testRoutinesDispatch)} >
                    <TimelineIcon color="primary" />
                </IconButton>
            </ButtonGroup>
        );
    }

    else if (
        row?.status === statusCodes.STANDBY
    ) {
        //console.log('==> STANDBY');
        return (
            <ButtonGroup>
                <Button size="small" sx={{ width: 72, p: 0.2, height: 32 }} onClick={() => showWizardsZero(row, ubaDevicesDispatch)}>
                    <Typography fontSize={11}>Start Test</Typography>
                </Button>
            </ButtonGroup>
        );
    } 

    else if (
        row?.status === statusCodes.PENDING
    ) {
        return (
            <ButtonGroup>
                <Button size="small" sx={{ width: 72, p: 0.2, height: 32 }} onClick={() => showWizardsZero(row, ubaDevicesDispatch)}>
                    <Typography fontSize={11}>Pending</Typography>
                </Button>
            </ButtonGroup>
        );
    }

    else if (
        row?.status === statusCodes.PENDING_STANDBY
    ) {
        return (
            <ButtonGroup>
                <Button size="small" sx={{ width: 72, p: 0.2, height: 32 }} onClick={() => showWizardsZero(row, ubaDevicesDispatch)}>
                    <Typography fontSize={11}>Pending Standby</Typography>
                </Button>

                <IconButton title={getText('common.STOP')} aria-label="stop" onClick={() => handleForceStopTest(row, authDispatch, ubaDevicesDispatch)} >
                    <StopCircleOutlinedIcon color="error" />
                </IconButton>
            </ButtonGroup>
        );
    }

    else if (
        row?.status === statusCodes.PENDING_STOP
    ) {
        return (
            <ButtonGroup>
                <Button size="small" sx={{ width: 72, p: 0.2, height: 32 }} onClick={() => showWizardsZero(row, ubaDevicesDispatch)}>
                    <Typography fontSize={11}>Pending Stop</Typography>
                </Button>

                <IconButton title={getText('common.STOP')} aria-label="stop" onClick={() => handleForceStopTest(row, authDispatch, ubaDevicesDispatch)} >
                    <StopCircleOutlinedIcon color="error" />
                </IconButton>
            </ButtonGroup>
        );
    }

    else if (
        row?.status === statusCodes.PENDING_RUNNING
    ) {
        return (
            <ButtonGroup>
                <Button size="small" sx={{ width: 72, p: 0.2, height: 32 }} onClick={() => showWizardsZero(row, ubaDevicesDispatch)}>
                    <Typography fontSize={11}>Pending Running</Typography>
                </Button>
                
                <IconButton title={getText('common.STOP')} aria-label="stop" onClick={() => handleForceStopTest(row, authDispatch, ubaDevicesDispatch)} >
                    <StopCircleOutlinedIcon color="error" />
                </IconButton>
            </ButtonGroup>
        );
    }

    else if (
        row?.status === statusCodes.PENDING_SAVE
    ) {
        return (
            <ButtonGroup>
                <Button size="small" sx={{ width: 72, p: 0.2, height: 32 }} onClick={() => showWizardsZero(row, ubaDevicesDispatch)}>
                    <Typography fontSize={11}>Pending Save</Typography>
                </Button>
            </ButtonGroup>
        );
    }

    else if (
        row?.status === statusCodes.PENDING_PAUSE
    ) {
        return (
            <ButtonGroup>
                <Button size="small" sx={{ width: 72, p: 0.2, height: 32 }} onClick={() => showWizardsZero(row, ubaDevicesDispatch)}>
                    <Typography fontSize={11}>Pending Pause</Typography>
                </Button>
            </ButtonGroup>
        );
    }

    else  {
        return (
            <ButtonGroup>
                <Button size="small" sx={{ width: 72, p: 0.2, height: 32 }}>
                    <Typography fontSize={11}>Unknown</Typography>
                </Button>
            </ButtonGroup>
        );
    }
};

const showWizardsZero = (selectedRow, ubaDevicesDispatch) => {
    ubaDevicesDispatch(setCurrentUba(selectedRow));
    ubaDevicesDispatch(setSelectedDevices([]));//not sure if this is needed
    ubaDevicesDispatch(setState(pageStateList.WIZARD_ZERO));
};

const handleStopTest = (selectedRow, authDispatch, ubaDevicesDispatch) => {
    let choice = true;//confirm('Confirm Stop Run');
    if(choice === true) {
    //console.log('==> handleStopTest');
        stopRunningTest(authDispatch, ubaDevicesDispatch, selectedRow?.runningTestID, selectedRow?.ubaSN, selectedRow?.testRoutineChannels);
        return true;
    }
    return false;
};

const handleForceStopTest = (selectedRow, authDispatch, ubaDevicesDispatch) => {
    let choice = true;//confirm('Confirm Stop Run');
    if(choice === true) {
    //console.log('==> handleForceStopTest');
        forceStopRunningTest(authDispatch, ubaDevicesDispatch, selectedRow?.runningTestID, selectedRow?.ubaSN, selectedRow?.testRoutineChannels);
        return true;
    }
    return false;
};

const handlePauseTest = (selectedRow, authDispatch, ubaDevicesDispatch) => {
    let choice = true;//confirm('Confirm Pause Run');
    if(choice === true) {
        pauseRunningTest(authDispatch, ubaDevicesDispatch, selectedRow?.runningTestID, selectedRow?.ubaSN, selectedRow?.testRoutineChannels);
        return true;
    }
    return false;
};

const handleNextTest = (selectedRow, authDispatch, ubaDevicesDispatch) => {
    let choice = true;//confirm('Confirm Next-Step');
    if(choice === true) {
        nextStepRunningTest(authDispatch, ubaDevicesDispatch, selectedRow?.runningTestID, selectedRow?.ubaSN, selectedRow?.testRoutineChannels);
        return true;
    }
    return false;
};

const handleResumeTest = (selectedRow, authDispatch, ubaDevicesDispatch) => resumeRunningTest(authDispatch, ubaDevicesDispatch, selectedRow?.runningTestID, selectedRow?.ubaSN, selectedRow?.testRoutineChannels);

const handleConfirmTest = (selectedRow, authDispatch, ubaDevicesDispatch) => confirmRunningTest(authDispatch, ubaDevicesDispatch, selectedRow?.runningTestID, selectedRow?.ubaSN, selectedRow?.testRoutineChannels);

const handleGraphOpening = (selectedRow, authDispatch, ubaDevicesDispatch, testRoutinesDispatch) => {
    getGraphData(authDispatch, testRoutinesDispatch, selectedRow.runningTestID);
    ubaDevicesDispatch(setCurrentUba(selectedRow));
    authDispatch(setModal('graph.details'));
};

