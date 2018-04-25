$(function() {
	$("form .initial-producers").repeatable({
		template: "#template_initial_producers",
		itemContainer: ".initial-producer",
		min: 21,
		max: 21
	});
	$("form .additional-accounts").repeatable({
		template: "#template_additional_accounts",
		itemContainer: ".additional-account",
		addTrigger: ".add-account"
	});
});
