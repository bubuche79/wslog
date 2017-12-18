function get_x(json) {
	if (json.period.from != null) {
		return json.data.length;
	} else if (json.period.month == null) {
		return 12;
	} else {
		return new Date(json.period.year, json.period.month, 0).getDate();
	}
};

function scale_min(a) {
	return 5 * parseInt((a + 4) / 5) - 5;
}

function scale_max(a) {
	return 5 * parseInt(a / 5) + 5;
}

function get_labels(json) {
	var labels = [];
	var x = get_x(json);

	for (var i = 0; i < x; i++) {
		if (json.period.from != null) {
			labels.push(json.data[i].time);
		} else if (json.period.month == null) {
			labels.push(i > 0 && i < x + 1 ? i : '');
		} else {
			labels.push(i > 0 && i < x + 1 && (i % 2 == 0) ? i : '');
		}
	}

	return labels;
}

function get_data(json, field) {
	var data = [];
	var x = get_x(json);
	var x_field = (json.period.from != null && json.period.month == null) ? 'month' : 'day';

	var j = 0;

	for (var i = 0; i < x; i++) {
		if (json.period.from != null) {
			data.push(json.data[j][field]);
			j++;
		} else if (j < json.data.length && json.data[j][x_field] == i) {
			data.push(json.data[j][field]);
			j++;
		} else {
			data.push(null);
		}
	}

	return data;
}

function get_type(config) {
	var type = 'line';
	var datasets = config.datasets;

	for (i = 0; i < datasets.length; i++) {
		if (datasets[i].type != 'line') {
			type = datasets[i].type;
		}
	}

	return type;
}

function create_chart(json, config) {
	var chartjs = {
		type: get_type(config),
		data: {
			labels: get_labels(json),
			datasets: [],
		},
		options: {
			title: {
				display: true,
				fontSize: 16,
				text: config.options.title
			},
			legend: {
				position: 'bottom'
			},
			tooltips: {
				mode: 'index',
				intersect: false,
				position: 'nearest',
				bodySpacing: 5,
				callbacks: {
					title: function(items, data) {
						return moment.unix(items[0].xLabel).format('DD/MM/YY HH:mm');
					},
					label: function(item, data) {
						var result = null;
						var dataset = data.datasets[item.datasetIndex];

						if (dataset.data[item.index] != null) {
							var unit = '';

							for (i = 0; i < config.options.axes.length; i++) {
								if (dataset.yAxisID == config.options.axes[i].id) {
									unit = config.options.axes[i].unit;
								}
							}

							result = dataset.label + ' : ' + item.yLabel.toFixed(1) + ' ' + unit;
						}

						return result;
					}
				}
			},
			scales: {
				xAxes: [{
					type: 'time',
					gridLines: {
						offsetGridLines: false
					},
					time: {
						unit: 'day',
						parser: moment.unix,
						min: new Date(json.period.from),
						displayFormats: {
							hour: 'HH:mm',
							day: 'D MMM'
						}
					},
					scaleLabel: {
						display: true,
//						labelString: x_label,
						fontStyle: 'bold'
					}
				}],
				yAxes: []
			}
		}
	}

	// Configure datasets
	for (i = 0; i < config.datasets.length; i++) {
		var dataset = config.datasets[i];

		var dat = {
			type: dataset.type,
			label: dataset.label,
			yAxisID: dataset.axis,
			backgroundColor: dataset.color,
			borderColor: dataset.color,
			data: get_data(json, dataset.field)
		};

		if (dataset.type == 'line') {
			dat.fill = false;
			dat.lineTension = 0;
			dat.borderWidth = 2;
			dat.pointStyle = dataset.pointStyle;
			dat.pointRadius = 0;
			dat.pointHitRadius = 5;
			dat.pointHoverRadius = 5;
			dat.spanGaps = false;
		}

		chartjs.data.datasets.push(dat);
	}

	// Configure axes
	for (i = 0; i < config.options.axes.length; i++) {
		var axes = config.options.axes[i];

		var dat = {
			type: 'linear',
			position: axes.position,
			id: axes.id,
			ticks: {
//				min: scale_min(temp_min.min),
//				max: scale_max(temp_max.max),
				stepSize: 5
			},
			scaleLabel: {
				display: true,
				labelString: axes.label + ' (' + axes.unit + ')',
				fontStyle: 'bold'
			}
		};

		chartjs.options.scales.yAxes.push(dat);
	}

	return chartjs;
}

function obs_temp(json) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Température',
			field: 'temp',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(237, 86, 27, 1)'
		},{
			type: 'line',
			label: 'Point de rosée',
			field: 'dew_point',
			axis: 'y-axis-1',
			pointStyle: 'rect',
			color: 'rgba(80, 180, 50, 1)'
		},{
			type: 'line',
			label: 'Humidité',
			field: 'humidity',
			axis: 'y-axis-2',
			color: 'rgba(5, 141, 199, 1)'
		}],
		options: {
			title: 'Températures, humidité, point de rosée',
			axes: [{
				id: 'y-axis-1',
				position: 'left',
				label: 'Température',
				unit: '°C'
			},{
				id: 'y-axis-2',
				position: 'right',
				label: 'Humidité',
				unit: '%'
			}]
		}
	};

	return create_chart(json, options);
}

function obs_wind(json) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Vent moyen',
			field: 'wind_speed',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(69, 114, 167, 1)'
		},{
			type: 'line',
			label: 'Direction',
			field: 'wind_dir',
			axis: 'y-axis-2',
			pointStyle: 'rect',
			color: 'rgba(170, 70, 70, 1)'
		}],
		options: {
			title: 'Vent',
			axes: [{
				id: 'y-axis-1',
				position: 'left',
				label: 'Vent',
				unit: 'm/s'
			},{
				id: 'y-axis-2',
				position: 'right',
				label: 'Direction',
				unit: '°'
			}]
		}
	};

	return create_chart(json, options);
}

function chart_wind(json) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Rafale',
			field: 'wind_gust_speed',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(86, 65, 112, 1)'
		},{
			type: 'line',
			label: 'Vent moyen',
			field: 'wind_speed',
			axis: 'y-axis-1',
			pointStyle: 'rect',
			color: 'rgba(128, 105, 155, 1)'
		}],
		options: {
			title: 'Vent',
			axes: [{
				id: 'y-axis-1',
				position: 'left',
				label: 'Vent',
				unit: 'm/s'
			}]
		}
	};

	return create_chart(json, options);
};

function chart_barometer(json) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Pression moyenne',
			field: 'barometer',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(137, 165, 78, 1)'
		}],
		options: {
			title: 'Pression au niveau de la mer',
			axes: [{
				id: 'y-axis-1',
				position: 'left',
				label: 'Pression',
				unit: 'hPa'
			}]
		}
	};

	return create_chart(json, options);
};

function chart_year_temp(json) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Température minimale',
			field: 'temp_min',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(69, 114, 167, 1)'
		},{
			type: 'line',
			label: 'Température maximale',
			field: 'temp_max',
			axis: 'y-axis-1',
			pointStyle: 'rect',
			color: 'rgba(170, 70, 70, 1)'
		}],
		options: {
			title: 'Températures',
			axes: [{
				id: 'y-axis-1',
				position: 'left',
				label: 'Températures',
				unit: '°C'
			}]
		}
	};

	return create_chart(json, options);
};

function chart_year_rain(json) {
	var options = {
		datasets: [{
			type: 'line',
			label: 'Précipitations maximales sur 24h',
			field: 'rain_24h',
			axis: 'y-axis-1',
			pointStyle: 'circle',
			color: 'rgba(18, 137, 186, 1)'
		}, {
			type: 'bar',
			label: 'Précipitations',
			field: 'rain',
			axis: 'y-axis-1',
			color: 'rgba(162, 190, 163, 1)'
		}],
		options: {
			title: 'Précipitations',
			axes: [{
				id: 'y-axis-1',
				position: 'left',
				label: 'Précipitations',
				unit: 'mm'
			}]
		}
	};

	return create_chart(json, options);
};

