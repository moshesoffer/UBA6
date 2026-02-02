export const getInputColor = (fieldName, newInputValue, initialTestRoutine, existingTest, warningColor, inputColors) => {
	let colorObject = null;

	if (existingTest.isTrue && newInputValue?.toString() !== initialTestRoutine?.[fieldName]?.toString()) {
		colorObject = {
			color: warningColor,
		};
	}

	return {
		...inputColors,
		[fieldName]: colorObject,
	};
};
