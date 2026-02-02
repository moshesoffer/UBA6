import {useEffect, useRef,} from 'react';

import Stack from '@mui/material/Stack';

import {useTestRoutines, useTestRoutinesDispatch} from 'src/store/TestRoutinesProvider';
import {useSettings} from 'src/store/SettingsProvider';
import {validateNumber, validateString,} from 'src/utils/validators';

import UpperPart from './UpperPart';
import LowerPart from './LowerPart';
import GapBox from '../../GapBox';
import CustomHead from './../CustomHead';
import {handleInputChange,} from '../../utils';

export default function Charge(props) {

	let refCell = useRef({});

	const {id, isCollapsed, minTemp, maxTemp, chargeCurrent, chargePerCell,} = props;
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
		if (validateNumber(testData?.maxVoltage) && chargePerCell === null) {
			const maxVoltage = testData.maxVoltage.toFixed(2);
			handleInputChange(testRoutinesDispatch, id, 'chargePerCell', maxVoltage, true);
		}
	}, [testData?.maxVoltage,]);

	useEffect(() => {
		let chargeCurrentValue = chargeCurrent?.split(':')?.[0];
		chargeCurrentValue = parseFloat(chargeCurrentValue);
		const ratedBatteryCapacity = parseFloat(testData?.ratedBatteryCapacity?.toString());
		if (!validateNumber(chargeCurrentValue) || !validateNumber(ratedBatteryCapacity)) {
			return;
		}
		
		const chargeCurrentType = chargeCurrent?.split(':')?.[1];
		let cRate;
		if(chargeCurrentType === 'absoluteMa'){
			cRate = chargeCurrentValue / ratedBatteryCapacity;
		} else if(chargeCurrentType === 'absoluteA'){
			cRate = (chargeCurrentValue * 1000) / ratedBatteryCapacity;
		} else if (chargeCurrentType === 'relative'){//in case of relative, cRate is actually equals to relative value. and this calculation is actually Current
			cRate = chargeCurrentValue * ratedBatteryCapacity;
		}

		cRate = cRate.toFixed(2);
		// Avoid 0.0 value.
		cRate = Number(cRate?.toString());

		handleInputChange(testRoutinesDispatch, id, 'cRate', cRate, true);
	}, [chargeCurrent, testData?.ratedBatteryCapacity,]);

	if (isCollapsed) {
		return (
			<>
				<CustomHead {...{id}} isCollapsed={isCollapsed} title="testEditor.CHARGE"/>
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
				<CustomHead {...{id}} isCollapsed={isCollapsed} title="testEditor.CHARGE"/>

				<UpperPart {...props} {...refCell.current}/>

				<LowerPart {...props} {...refCell.current}/>
			</Stack>

			<GapBox {...{id}}/>
		</>
	);
}
