function days(year, month) {
	return new Date(year, month, 0).getDate();
};

function min(a, b) {
	return (a == null || b < a) ? b : a;
}

function max(a, b) {
	return (a == null || a < b) ? b : a;
}

function scale_min(a) {
	return 5 * parseInt((a + 4) / 5) - 5;
}

function scale_max(a) {
	return 5 * parseInt(a / 5) + 5;
}

function chart1(json) {
	var labels = [];
	var datasets = {
		temp_min: [],
		temp_max: [],
		rain: []
	};
	var limits = {
		temp_min: null,
		temp_max: null,
		rain: null
	};

	var j = 0;
	var ndays = days(json.period.year, json.period.month);

	for (var i = 1; i < ndays + 1; i++) {
		labels.push(i);

		if (j < json.data.length && json.data[j].day == i) {
			datasets.temp_min.push(json.data[j].temp_min);
			datasets.temp_max.push(json.data[j].temp_max);
			datasets.rain.push(json.data[j].rain);

			limits.temp_min = min(limits.temp_min, json.data[j].temp_min);
			limits.temp_max = max(limits.temp_max, json.data[j].temp_max);
			limits.rain = max(limits.rain, json.data[j].rain);

			j++;
		} else {
			datasets.temp_min.push(null);
			datasets.temp_max.push(null);
			datasets.rain.push(null);
		}
	}

	var options = {
		type: 'bar',
		data: {
			labels: labels,
			datasets: [{
				type: 'line',
				label: 'Température minimale',
				fill: false,
				yAxisID: 'temp',
				lineTension: 0,
				backgroundColor: 'rgba(69, 114, 167, 1)',
				borderColor: 'rgba(69, 114, 167, 1)',
				pointStyle: 'circle',
				data: datasets.temp_min
			}, {
				type: 'line',
				label: 'Température maximale',
				fill: false,
				yAxisID: 'temp',
				lineTension: 0,
				backgroundColor: 'rgba(170, 70, 70, 1)',
				borderColor: 'rgba(170, 70, 70, 1)',
				pointStyle: 'rectRounded',
				data: datasets.temp_max
			}, {
				type: 'bar',
				label: 'Pluie',
				yAxisID: 'rain',
				backgroundColor: 'rgba(162, 190, 163, 1)',
				borderColor: 'rgba(162, 190, 163, 1)',
				data: datasets.rain
			}]
		},
		options: {
			title: {
				display: true,
				text: 'Températures extrêmes, précipitations'
			},
			legend: {
				position: 'top'
			},
			tooltips: {
				mode: 'x',
				intersect: false,
				position: 'nearest'
			}, 
			scales: {
				yAxes: [{
					type: 'linear',
					position: 'left',
					id: 'temp',
					ticks: {
						min: scale_min(limits.temp_min),
						max: scale_max(limits.temp_max)
					},
					scaleLabel: {
						display: true,
						labelString: "Températures (°C)",
						fontStyle: 'bold'
					}
				}, {
					type: 'linear',
					position: 'right',
					id: 'rain',
					ticks: {
						min: 0,
						max: scale_max(limits.rain)
					},
					scaleLabel: {
						display: true,
						labelString: "Pluie (mm)",
						fontStyle: 'bold'
					}
				}]
			}
		}
	}

	return options;
};

