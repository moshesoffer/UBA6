import {useEffect, useRef,} from 'react';

import Stack from '@mui/material/Stack';

import {useTestRoutines, useTestRoutinesDispatch} from 'src/store/TestRoutinesProvider';
import {useSettings} from 'src/store/SettingsProvider';
import {validateInteger, validateNumber, validateString,} from 'src/utils/validators';

import UpperPart from './UpperPart';
import LowerPart from './LowerPart';
import GapBox from '../../GapBox';
import CustomHead from './../CustomHead';
import {handleInputChange,} from '../../utils';

export default function Discharge(props) {

	let refCell = useRef({});

	const {id, isCollapsed, minTemp, maxTemp, cutOffVoltage,dischargeCurrent} = props;
	const {testData,} = useTestRoutines();
	const {cells,} = useSettings();

	const testRoutinesDispatch = useTestRoutinesDispatch();

	useEffect(() => {
		if (validateString(testData?.cellPN)) {
			const currentCell = cells.find(cell => cell.itemPN === testData.cellPN);
			refCell.current = {
				...currentCell,
				minTempDefault: currentCell?.minTemp,
				maxTempDefault: currentCell?.maxTemp,
			}
			delete refCell.current.minTemp;
			delete refCell.current.maxTemp;

			if (minTemp === null) {
				handleInputChange(testRoutinesDispatch, id, 'minTemp', refCell.current?.minTempDefault, true);
			}
			if (maxTemp === null) {
				handleInputChange(testRoutinesDispatch, id, 'maxTemp', refCell.current?.maxTempDefault, true);
			}
		}
	}, [testData?.cellPN,]);

	useEffect(() => {
		const noCellSerial = parseInt(testData?.noCellSerial?.toString());
		if (validateString(testData?.cellPN) && validateInteger(noCellSerial) && noCellSerial > 0) {
			const chosenCellPN = cells.find(cell => cell.itemPN === testData.cellPN);
			const minVoltage = chosenCellPN?.minVoltage;

			if (validateNumber(minVoltage) && minVoltage > 0 && cutOffVoltage === null) {
				const cutOffVoltage = noCellSerial * minVoltage;
				handleInputChange(testRoutinesDispatch, id, 'cutOffVoltage', cutOffVoltage, true);
			}
		}
	}, [testData?.cellPN, testData?.noCellSerial,]);

	useEffect(() => {
		let dischargeCurrentValue = dischargeCurrent?.split(':')?.[0];
		dischargeCurrentValue = parseFloat(dischargeCurrentValue);
		const ratedBatteryCapacity = parseFloat(testData?.ratedBatteryCapacity?.toString());
		if (!validateNumber(dischargeCurrentValue) || !validateNumber(ratedBatteryCapacity)) {
			return;
		}
		//console.log('ratedBatteryCapacity', ratedBatteryCapacity);
		const dischargeCurrentType = dischargeCurrent?.split(':')?.[1];
		let cRate;
		if(dischargeCurrentType === 'absoluteMa'){
			cRate = dischargeCurrentValue / ratedBatteryCapacity;
		} else if(dischargeCurrentType === 'absoluteA'){
			cRate = (dischargeCurrentValue * 1000) / ratedBatteryCapacity;
		} else if (dischargeCurrentType === 'relative'){
			cRate = ratedBatteryCapacity * dischargeCurrentValue;//in case of relative, cRate is actually equals to relative value. and this calculation is actually Current 
		} else if (dischargeCurrentType === 'power'){
			cRate = (dischargeCurrentValue / testData?.maxPerBattery) / (ratedBatteryCapacity / 1000);
		} else {
			handleInputChange(testRoutinesDispatch, id, 'cRate', null, true);
			return;//resistance is not used in cRate calculation
		}

		cRate = cRate.toFixed(2);
		// Avoid 0.0 value.
		cRate = Number(cRate?.toString());

		handleInputChange(testRoutinesDispatch, id, 'cRate', cRate, true);
	}, [dischargeCurrent, testData?.ratedBatteryCapacity,]);

	if (isCollapsed) {
		return (
			<>
				<CustomHead {...{id}} isCollapsed={isCollapsed} title="testEditor.DISCHARGE"/>
				<GapBox {...{id}}/>
			</>
		);
	}

	return (
		<>
			<Stack spacing={1} sx={{
				width: 1,
				p: 1,
				border: 'solid 2px #d0d8e5',
				borderRadius: '16px',
			}}>
				<CustomHead {...{id}} isCollapsed={isCollapsed} title="testEditor.DISCHARGE"/>

				<UpperPart {...props} {...refCell.current}/>

				<LowerPart {...props} {...refCell.current}/>
			</Stack>

			<GapBox {...{id}}/>
		</>
	);
}
