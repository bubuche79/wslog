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

function tt_title(json, items) {
	return new Date(json.period.year, json.period.month, items[0].index).toLocaleString();
}

function tt_label(item, data) {
	return data.datasets[item.datasetIndex].label + ' : ' + item.yLabel + ' °C';
}

function aggr_labels(json) {
	var labels = [];

	var j = 0;
	var ndays = days(json.period.year, json.period.month);

	for (var i = 0; i < ndays + 2; i++) {
		labels.push(i > 0 && i < ndays + 1 && (i % 2 == 0) ? i : '');
	}

	return labels;
}

function aggr_dataset(json, field) {
	var dataset = {
		data: [],
		min: null,
		max: null
	};

	var j = 0;
	var ndays = days(json.period.year, json.period.month);

	for (var i = 0; i < ndays + 2; i++) {
		if (j < json.data.length && json.data[j].day == i) {
			var v = json.data[j][field]

			dataset.data.push(v);
			dataset.min = min(dataset.min, v);
			dataset.max = max(dataset.max, v);

			j++;
		} else {
			dataset.data.push(null);
		}
	}

	return dataset;
}

function chart_temp_rain(json) {
	var labels = aggr_labels(json);
	var temp_min = aggr_dataset(json, 'temp_min');
	var temp_max = aggr_dataset(json, 'temp_max');
	var rain = aggr_dataset(json, 'rain');

	var tmin_color = 'rgba(69, 114, 167, 1)';
	var tmax_color = 'rgba(170, 70, 70, 1)';
	var rain_color = 'rgba(162, 190, 163, 1)';

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
				backgroundColor: tmin_color,
				borderColor: tmin_color,
				borderWidth: 2,
				pointStyle: 'circle',
				data: temp_min.data
			}, {
				type: 'line',
				label: 'Température maximale',
				fill: false,
				yAxisID: 'temp',
				lineTension: 0,
				backgroundColor: tmax_color,
				borderColor: tmax_color,
				borderWidth: 2,
				pointStyle: 'rect',
				data: temp_max.data
			}, {
				type: 'bar',
				label: 'Pluie',
				yAxisID: 'rain',
				backgroundColor: rain_color,
				borderColor: rain_color,
				data: rain.data
			}]
		},
		options: {
			title: {
				display: true,
				fontSize: 16,
				text: 'Températures extrêmes, précipitations'
			},
			legend: {
				position: 'bottom'
			},
			tooltips: {
				mode: 'x',
				intersect: false,
				position: 'nearest',
				bodySpacing: 5,
				callbacks: {
					title: function(items, data) { return tt_title(json, items); },
					label: function(item, data) { return tt_label(item, data); }
				}
			}, 
			scales: {
				xAxes: [{
//					barThickness: 15,
					gridLines: {
						offsetGridLines: false,
					},
					ticks: {
						maxRotation: 0
					},
					scaleLabel: {
						display: true,
						labelString: "Jour du mois",
						fontStyle: 'bold'
					}
				}],
				yAxes: [{
					type: 'linear',
					position: 'left',
					id: 'temp',
					ticks: {
						min: scale_min(temp_min.min),
						max: scale_max(temp_max.max),
						stepSize: 5
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
						max: scale_max(rain.max) + 5,
						stepSize: 5
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

function chart_wind(json)
{
	var labels = aggr_labels(json);
	var wind = aggr_dataset(json, 'wind_speed');
	var wind_gust = aggr_dataset(json, 'wind_gust_speed');

	var wcolor = 'rgba(86, 65, 112, 1)';
	var wgcolor = 'rgba(128, 105, 155, 1)';

	var options = {
		type: 'line',
		data: {
			labels: labels,
			datasets: [{
				label: 'Vent moyen',
				fill: false,
				yAxisID: 'y-axis-1',
				lineTension: 0,
				backgroundColor: wgcolor,
				borderColor: wgcolor,
				borderWidth: 2,
				pointStyle: 'circle',
				data: wind.data
			}, {
				label: 'Rafale',
				fill: false,
				yAxisID: 'y-axis-1',
				lineTension: 0,
				backgroundColor: wcolor,
				borderColor: wcolor,
				borderWidth: 2,
				pointStyle: 'rect',
				data: wind_gust.data
			}]
		},
		options: {
			title: {
				display: true,
				fontSize: 16,
				text: 'Vent'
			},
			legend: {
				position: 'bottom'
			},
			tooltips: {
				mode: 'x',
				intersect: false,
				position: 'nearest',
				bodySpacing: 5,
				callbacks: {
					title: function(items, data) { return tt_title(json, items); },
					label: function(item, data) { return tt_label(item, data); }
				}
			}, 
			scales: {
				xAxes: [{
//					barThickness: 15,
					gridLines: {
						offsetGridLines: false,
					},
					ticks: {
						maxRotation: 0
					},
					scaleLabel: {
						display: true,
						labelString: "Jour du mois",
						fontStyle: 'bold'
					}
				}],
				yAxes: [{
					type: 'linear',
					position: 'left',
					id: 'y-axis-1',
					ticks: {
						min: 0,
						//max: scale_max(data.dataset.max),
						//stepSize: 1
					},
					scaleLabel: {
						display: true,
						labelString: "Vent (m/s)",
						fontStyle: 'bold'
					}
				}]
			}
		}
	}

	return options;
};
